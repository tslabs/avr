library AVR309;
{
  Author: Ing. Igor Cesko
          http://www.cesko.host.sk
          cesko@internet.sk
  Project: AVR309 application note : USB to UART converter
           www.atmel.com
}

//{$DEFINE DemoVersion}
  {$WARN UNSAFE_CODE OFF}
  {$WARN UNSAFE_TYPE OFF}
  {$WARN UNSAFE_CAST OFF}

uses
  Windows,Mmsystem,Messages;
{$R *.RES} //resource: AVRUSBicon
const
  GlobalRS232BufferHiIndexLength=100016; //length of RS232 buffer
type
  TGlobalRS232Buffer=array[0..GlobalRS232BufferHiIndexLength]of byte;
const
  DrvName:PChar='\\.\AVR309USB_0'; //name of driver IgorPlug.sys
  RS232BufferFileMappedName='IgorPlugUSBRS232FileMappedHandleName'; //name of unique memory mapped file - RS232 buffer
  RS232BufferMutexName='IgorPlugUSBRS232MutexHandleName'; //name of unique mutex receiving RS232 thread
  SerializationEventName='IgorPlugDLLEvent'; //name of unique event for serialization access to hardware
  UARTx2Mode:boolean=false;

const
  PGlobalRS232Buffer:^TGlobalRS232Buffer=nil; //pointer to RS232 buffer (unique in system - memory maped file)
  GlobalRS232BufferLength=SizeOf(TGlobalRS232Buffer); //size of RS232 buffer
  RS232BufferFileMappedHND:THandle=0;//file mapped handle of RS232 buffer
  RS232BufferWrite:integer=0;//write position in RS232 buffer
  RS232BufferRead:integer=0;//read position in RS232 buffer
  GlobalRS232BufferEnd=High(TGlobalRS232Buffer)-16;//end of write position in RS232 buffer (can be overrun up to 8 bytes)
  GlobalRS232BufferWritePos:integer=GlobalRS232BufferHiIndexLength-SizeOf(GlobalRS232BufferWritePos); //position in RS232 buffer where is located write position in RS232 buffer (for other applications to know where buffer ends)

  IsRS232PrevInst: Boolean=true; //if there is previous application with DLL using
  RS232MutexHandle: THandle=0; //handle of unique mutex - for detection if exist another application using this DLL

  DrvHnd:DWORD=INVALID_HANDLE_VALUE; //handle of driver IgorPlug.sys
  OutputDataLength=256;
var
  OutputData:array[0..OutputDataLength-1] of byte; //data to USB
const
  OutLength:integer=0; //length of data to USB

  DoGetRS232BufferTimer:integer=0; //handle of timer generating event: reading to RS232 from device FIFO in intervals
  RS232BufferTimerInterval:integer=10; //interval for reading from device FIFO into RS232 buffer
  RS232BufferThreadId:integer=0; //reading RS232 buffer thread ID
  RS232BufferThreadHND:integer=0; //reading RS232 buffer thread handle
  RS232BufferGetEvent:integer=0; //event - signal for read from device FIFO
  RS232BufferEndEvent:integer=0; //event to stop cyclic reading RS232 buffer thread
  LocalThreadRunningEvent:integer=0; //event to signal to end of local RS232 buffer thread


  SerializationEvent:THandle=0; //handle of serialization event - for serialization access to hardware
  SerializationEventTimeout:integer=20000;//20 seconds to wait to end previous device access

  HookHandle:integer=0; //hook handle: DLL is hooking message WM_QUIT to safe resouce unallocation

  {$IFDEF DemoVersion}
    DemoOK:integer=IDNO; //if there was clicked OK in DEMO version (in registered version is this IDOK)
    OKDemoValue:integer=IDOK+1; //for hackers debugging - comparing value with DemoOK
  {$ENDIF}


  //Function numbers in hardware device
  FNCNumberDoSetInfraBufferEmpty	=1	;//;restart of infra reading (if was stopped by RAM reading)
  FNCNumberDoGetInfraCode		=2	;//;transmit of receved infra code (if some code in infra buffer)
  FNCNumberDoSetDataPortDirection	=3	;//;set direction of data bits
  FNCNumberDoGetDataPortDirection	=4	;//;get direction of data bits
  FNCNumberDoSetOutDataPort		=5	;//;set data bits (if are bits as input, then set theirs pull-ups)
  FNCNumberDoGetOutDataPort		=6	;//;get data bits (if are bits as input, then get theirs pull-ups)
  FNCNumberDoGetInDataPort		=7	;//;get data bits - pin reading
  FNCNumberDoEEPROMRead			=8	;//;read EEPROM from given address
  FNCNumberDoEEPROMWrite		=9	;//;write data byte to EEPROM to given address
  FNCNumberDoRS232Send  		=10	;//;send one data byte to RS232 line
  FNCNumberDoRS232Read			=11	;//;read one data byte from serial line (only when FIFO not implemented in device)
  FNCNumberDoSetRS232Baud		=12	;//;set baud speed of serial line
  FNCNumberDoGetRS232Baud		=13     ;//;get baud speed of serial line (exact value)
  FNCNumberDoGetRS232Buffer		=14     ;//;get received bytes from FIFO RS232 buffer
  FNCNumberDoSetRS232DataBits           =15     ;//;set number of data bits of serial line
  FNCNumberDoGetRS232DataBits           =16     ;//;get number of data bits of serial line
  FNCNumberDoSetRS232Parity             =17     ;//;set parity of serial line
  FNCNumberDoGetRS232Parity             =18     ;//;get parity of serial line
  FNCNumberDoSetRS232StopBits           =19     ;//;set stopbits of serial line
  FNCNumberDoGetRS232StopBits           =20     ;//;get stopbits of serial line

  //results - answers from device
  NO_ERROR=0;
  DEVICE_NOT_PRESENT=1;
  NO_DATA_AVAILABLE=2;
  INVALID_BAUDRATE=3;
  OVERRUN_ERROR=4;
  INVALID_DATABITS=5;
  INVALID_PARITY=6;
  INVALID_STOPBITS=7;

//-------------------------------------------------------------------------------------------------------
function DoGetRS232Baud(var BaudRate:integer):integer;  stdcall export; forward;
//-------------------------------------------------------------------------------------------------------
function FNHookProc(code:Integer; wparam:Integer; lparam:Integer): Longint stdcall;
// Hook function: only when application ends then safely frees FIFO reading thread
var
  lpMsg:^TMsg;
begin
  lpMsg:=pointer(lparam);
  Result:=CallNextHookEx(HookHandle,code,wParam,lParam);
  if (lpMsg^.message=WM_QUIT) then
    begin
      if DoGetRS232BufferTimer<>0 then
        begin
          timeKillEvent(DoGetRS232BufferTimer);
          DoGetRS232BufferTimer:=0;
        end;
      SetEvent(RS232BufferEndEvent);
      WaitForSingleObject(RS232BufferThreadHND,5000);
      CloseHandle(RS232BufferThreadHND);
      RS232BufferThreadHND:=0;
    end;
end;
//-------------------------------------------------------------------------------------------------------
procedure ShowThreadErrorMessage;
//if some thread locks serialization access - display message
const
  InProgress:boolean=false;
begin
  SetEvent(SerializationEvent);
  if InProgress then Exit;
  InProgress:=true;
  MessageBox(0,'Another thread is using long time AVR309.dll','Error',MB_OK or MB_ICONERROR or MB_TASKMODAL or MB_TOPMOST);
  InProgress:=false;
end;
//-------------------------------------------------------------------------------------------------------
function OpenDriver:boolean;
//open device driver
  begin
    Result:=false;
    DrvHnd:=CreateFile(PChar(DrvName),GENERIC_WRITE or GENERIC_READ, FILE_SHARE_WRITE or FILE_SHARE_READ,
               nil,OPEN_EXISTING,0,0);
    if DrvHnd=INVALID_HANDLE_VALUE then Exit;
    Result:=true;
  end;
//-------------------------------------------------------------------------------------------------------
function CloseDriver:boolean;
//close device driver
  begin
    Result:=true;
    if DrvHnd<>INVALID_HANDLE_VALUE then CloseHandle(DrvHnd);
  end;
//-------------------------------------------------------------------------------------------------------
function SendToDriver(FunctionNumber:byte; Param1,Param2:word;
                      var OutputData:array of byte; var OutLength:integer):boolean;
//send and receive data to device through device driver
  var
    LocalInBuffer:array [0..4] of byte;
    RepeatCount:integer;
    OutLengthMax:integer;
  begin
    {$IFDEF DemoVersion}
      if abs(DemoOK)<>(Round(OKDemoValue)-1) then
        WaitForSingleObject(SerializationEvent,SerializationEventTimeout);//delay for bad cracking
    {$ENDIF}
    RepeatCount:=3;
    Result:=false;
    if not OpenDriver then
      begin
        OutLength:=0;
        Exit;
      end;
    LocalInBuffer[0]:=FunctionNumber;
    LocalInBuffer[1]:=Lo(Param1);
    LocalInBuffer[2]:=Hi(Param1);
    LocalInBuffer[3]:=Lo(Param2);
    LocalInBuffer[4]:=Hi(Param2);
    if OutLength<0 then
      OutLengthMax:=OutputDataLength
    else
      OutLengthMax:=OutLength;
    if OutLengthMax>255 then
      OutLengthMax:=255;
    try
      repeat
        Result:=DeviceIoControl(DrvHnd,$800+8,@LocalInBuffer,5,@OutputData,OutLengthMax,cardinal(OutLength),nil);
        Result:=Result and (OutLength>0);
        dec(RepeatCount);
      until (OutLength>0)or(RepeatCount<=0);
    except
      DrvHnd:=CreateFile(PChar(DrvName),GENERIC_WRITE or GENERIC_READ, FILE_SHARE_WRITE or FILE_SHARE_READ,nil,OPEN_EXISTING,0,0);
    end;
    CloseDriver;
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetInfraBufferEmpty:integer;  stdcall export;
//;restart of infra reading (if was stopped by RAM reading)
  begin
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetInfraBufferEmpty,0,0,OutputData,OutLength) then
      Result:=NO_ERROR;
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetInfraCode(var TimeCodeDiagram:array of byte; var DiagramLength:integer):integer;  stdcall export;
//;transmit of receved infra code (if some code in infra buffer)
  const
    LastReadedCode:integer=-1;
    HeaderLength=3;
  var
    BytesToRead:integer;
    OutputData:array[0..200]of byte;
    LastWrittenIndex:integer;
    i,j,k:integer;
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    DiagramLength:=0;
    OutLength:=3;  //3 bytes is IR header length
    if not SendToDriver(FNCNumberDoGetInfraCode,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    BytesToRead:=OutputData[0];
    if (LastReadedCode=OutputData[1])or(OutLength<=1)or(BytesToRead=0) then
      begin
        Result:=NO_ERROR;
        SetEvent(SerializationEvent);
        Exit;
      end;
    LastReadedCode:=OutputData[1];
    LastWrittenIndex:=OutputData[2];
    i:=0;
    while i<BytesToRead do
      begin
        OutLength:=BytesToRead-i;
        if not SendToDriver(FNCNumberDoGetInfraCode,i+HeaderLength,0,OutputData[i],OutLength) then
          begin
            DoSetInfraBufferEmpty;
            LastReadedCode:=-1;
            SetEvent(SerializationEvent);
            Exit;
          end;
        inc(i,OutLength);
      end;
    j:=LastWrittenIndex mod BytesToRead;
    k:=0;
    for i:=j to BytesToRead-1 do
      begin
        TimeCodeDiagram[k]:=OutputData[i];
        inc(k);
      end;
    for i:=0 to (j-1) do
      begin
        TimeCodeDiagram[k]:=OutputData[i];
        inc(k);
      end;
    DiagramLength:=BytesToRead;
    DoSetInfraBufferEmpty;
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetDataPortDirections(DirectionByteB, DirectionByteC, DirectionByteD:byte; UsedPorts:byte):integer;  stdcall export;
//;set direction of data bits
var
  Param1,Param2:word;
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    Param1:=DirectionByteC;
    Param1:=(Param1 shl 8) or DirectionByteB;
    Param2:=UsedPorts;
    Param2:=(Param2 shl 8) or DirectionByteD;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetDataPortDirection,Param1,Param2,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetDataPortDirection(DirectionByte:byte):integer;  stdcall export;
//;set direction of data bits (all ports the same)
  begin
    Result:=DoSetDataPortDirections(DirectionByte,DirectionByte,DirectionByte,$FF);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetDataPortDirections(var DataDirectionByteB, DataDirectionByteC, DataDirectionByteD:byte; var UsedPorts:byte):integer;  stdcall export;
//;get direction of data bits
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=3;
    if not SendToDriver(FNCNumberDoGetDataPortDirection,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    UsedPorts:=0;
    if (OutLength>0)and(@DataDirectionByteB<>nil) then
      begin
        DataDirectionByteB:=OutputData[0];
        UsedPorts:=UsedPorts or 1;
      end;
    if (OutLength>1)and(@DataDirectionByteC<>nil) then
      begin
        DataDirectionByteC:=OutputData[1];
        UsedPorts:=UsedPorts or 2;
      end;
    if (OutLength>2)and(@DataDirectionByteD<>nil) then
      begin
        DataDirectionByteD:=OutputData[2];
        UsedPorts:=UsedPorts or 4;
      end;
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetDataPortDirection(var DataDirectionByte:byte):integer;  stdcall export;
//;get direction of data bits
  var
    Dummy:byte;
  begin
    Result:=DoGetDataPortDirections(DataDirectionByte,Dummy,Dummy,Dummy);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetOutDataPorts(DataOutByteB, DataOutByteC, DataOutByteD:byte; UsedPorts:byte):integer;  stdcall export;
//;set data bits (if are bits as input, then set theirs pull-ups)
var
  Param1,Param2:word;
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    Param1:=DataOutByteC;
    Param1:=(Param1 shl 8) or DataOutByteB;
    Param2:=UsedPorts;
    Param2:=(Param2 shl 8) or DataOutByteD;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetOutDataPort,Param1,Param2,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetOutDataPort(DataOutByte:byte):integer;  stdcall export;
//;set data bits (if are bits as input, then set theirs pull-ups)
  begin
    Result:=DoSetOutDataPorts(DataOutByte,DataOutByte,DataOutByte,$FF);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetOutDataPorts(var DataOutByteB, DataOutByteC, DataOutByteD:byte; var UsedPorts:byte):integer;  stdcall export;
//;get data bits (if are bits as input, then get theirs pull-ups)
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=3;
    if not SendToDriver(FNCNumberDoGetOutDataPort,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    UsedPorts:=0;
    if (OutLength>0)and(@DataOutByteB<>nil) then
      begin
        DataOutByteB:=OutputData[0];
        UsedPorts:=UsedPorts or 1;
      end;
    if (OutLength>1)and(@DataOutByteC<>nil) then
      begin
        DataOutByteC:=OutputData[1];
        UsedPorts:=UsedPorts or 2;
      end;
    if (OutLength>2)and(@DataOutByteD<>nil) then
      begin
        DataOutByteD:=OutputData[2];
        UsedPorts:=UsedPorts or 4;
      end;
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetOutDataPort(var DataOutByte:byte):integer;  stdcall export;
//;get data bits (if are bits as input, then get theirs pull-ups)
  var
    Dummy:byte;
  begin
    Result:=DoGetOutDataPorts(DataOutByte,Dummy,Dummy,Dummy);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetInDataPorts(var DataInByteB, DataInByteC, DataInByteD:byte; var UsedPorts:byte):integer;  stdcall export;
//;get data bits - pin reading
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=3;
    if not SendToDriver(FNCNumberDoGetInDataPort,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    UsedPorts:=0;
    if (OutLength>0)and(@DataInByteB<>nil) then
      begin
        DataInByteB:=OutputData[0];
        UsedPorts:=UsedPorts or 1;
      end;
    if (OutLength>1)and(@DataInByteC<>nil) then
      begin
        DataInByteC:=OutputData[1];
        UsedPorts:=UsedPorts or 2;
      end;
    if (OutLength>2)and(@DataInByteD<>nil) then
      begin
        DataInByteD:=OutputData[2];
        UsedPorts:=UsedPorts or 4;
      end;
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetInDataPort(var DataInByte:byte):integer;  stdcall export;
//;get data bits - pin reading
  var
    Dummy:byte;
  begin
    Result:=DoGetInDataPorts(DataInByte,Dummy,Dummy,Dummy);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetRS232DataBits(DataBits:byte):integer;  stdcall export;
//;set databits number of RS232
  begin
    if not(DataBits in [5,6,7,8]) then
      begin
        Result:=INVALID_DATABITS;
        Exit;
      end;
    DataBits:=DataBits-5;
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetRS232DataBits,DataBits,0,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232DataBits(var DataBits:byte):integer;  stdcall export;
//;get databits number of RS232
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if not SendToDriver(FNCNumberDoGetRS232DataBits,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    DataBits:=OutputData[0]+5;
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetRS232Parity(Parity:byte):integer;  stdcall export;
//;set parity of RS232
  const
    ParityNone		=0;
    ParityOdd		=1;
    ParityEven		=2;
    ParityMark		=3;
    ParitySpace		=4;
  begin
    if not(Parity in [ParityNone,ParityOdd,ParityEven,ParityMark,ParitySpace]) then
      begin
        Result:=INVALID_PARITY;
        Exit;
      end;
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetRS232Parity,Parity,0,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232Parity(var Parity:byte):integer;  stdcall export;
//;get parity of RS232
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if not SendToDriver(FNCNumberDoGetRS232Parity,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    Parity:=OutputData[0];
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetRS232StopBits(StopBits:byte):integer;  stdcall export;
//;set stopbits of RS232
  const
    StopBit1		=0;
    StopBit2		=1;
  begin
    if not(StopBits in [StopBit1,StopBit2]) then
      begin
        Result:=INVALID_STOPBITS;
        Exit;
      end;
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetRS232StopBits,StopBits,0,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232StopBits(var StopBits:byte):integer;  stdcall export;
//;get stopbits of RS232
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if not SendToDriver(FNCNumberDoGetRS232StopBits,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    StopBits:=OutputData[0];
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoEEPROMRead(Address:word; var DataInByte:byte):integer;  stdcall export;
//;read EEPROM from given address
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if not SendToDriver(FNCNumberDoEEPROMRead,Address,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    DataInByte:=OutputData[0];
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoEEPROMWrite(Address:word; DataOutByte:byte):integer;  stdcall export;
//;write data byte to EEPROM to given address
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoEEPROMWrite,Address,DataOutByte,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoRS232Send(DataOutByte:byte):integer;  stdcall export;
//;send one data byte to RS232 line
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoRS232Send,DataOutByte,0,OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoRS232Read(var DataInByte:byte):integer;  stdcall export;
//;read one data byte from serial line (only when FIFO not implemented in device)
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=3;
    if not SendToDriver(FNCNumberDoRS232Read,0,0,OutputData,OutLength)then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    if OutLength=2 then
      begin
        Result:=NO_DATA_AVAILABLE;
        SetEvent(SerializationEvent);
        Exit;
      end;
    Result:=NO_ERROR;
    if OutLength=3 then
      begin
        Result:=OVERRUN_ERROR;
      end;
    DataInByte:=OutputData[0];
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoSetRS232Baud(BaudRate:integer):integer;  stdcall export;
//;set baud speed of serial line
//BAUD=12e6/(16*(WORD+1))
  var
    BaudRateByte:word;
    BaudRateDouble:double;
    BaudError:double;
    MaxBaudRateByte:integer;
  const
    Error=0.04; //4% max chyba
    MaxError=1+Error;
    MinError=1-Error;
  begin
    DoGetRS232Baud(MaxBaudRateByte);
    if UARTx2Mode then
      begin
        MaxBaudRateByte:=4095;
        BaudRate:=Round(BaudRate/2);
      end
    else
      begin
        MaxBaudRateByte:=255;
      end;
    Result:=INVALID_BAUDRATE;
    if BaudRate >= 12e6/16*MaxError then Exit;
    if BaudRate <= 12e6/(16*(MaxBaudRateByte+1))*MinError then Exit;
    BaudRateDouble:=Round(12e6/(16*BaudRate)-1);
    if BaudRateDouble<0 then BaudRateDouble:=0;
    if BaudRateDouble>MaxBaudRateByte then BaudRateDouble:=MaxBaudRateByte;
    BaudError:=12e6/(16*(BaudRateDouble+1))/BaudRate;
    if BaudError>MaxError then Exit;
    if BaudError<MinError then Exit;
    BaudRateByte:=Round(BaudRateDouble);

    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=1;
    if SendToDriver(FNCNumberDoSetRS232Baud,Lo(BaudRateByte),Hi(BaudRateByte),OutputData,OutLength) then
      Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232Baud(var BaudRate:integer):integer;  stdcall export;
//;get baud speed of serial line (exact value)
  begin
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    OutLength:=2;
    if not SendToDriver(FNCNumberDoGetRS232Baud,0,0,OutputData,OutLength) then
      begin
        SetEvent(SerializationEvent);
        Exit;
      end;
    UARTx2Mode:=OutLength>1;
    if not UARTx2Mode then OutputData[1]:=0; // ATmega returns 2 byte answer
    BaudRate:=Round(12e6/(16*(255.0*OutputData[1]+OutputData[0]+1.0)));
    if UARTx2Mode then BaudRate:=BaudRate*2; // ATmega has x2 mode on UART
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232BufferLocal(var RS232Buffer:array of byte; var RS232BufferLength:integer):integer;
//local function to obtain FIFO buffer into buffer
var
  i,j:integer;
  HeaderLength:integer;
  begin
    if RS232BufferLength<=0 then
      begin
        RS232BufferLength:=0;
        Result:=NO_ERROR;
        Exit;
      end;
    if WaitForSingleObject(SerializationEvent,SerializationEventTimeout)=WAIT_TIMEOUT then
      begin
        ShowThreadErrorMessage;
        Result:=DEVICE_NOT_PRESENT;
        RS232BufferLength:=0;
        Exit;
      end;
    Result:=DEVICE_NOT_PRESENT;
    i:=0;
    HeaderLength:=1;
    if UARTx2Mode then inc(HeaderLength);
    repeat
      OutLength:=RS232BufferLength-i+HeaderLength;
      if OutLength>OutputDataLength then
        OutLength:=OutputDataLength;
      if not SendToDriver(FNCNumberDoGetRS232Buffer,0,0,OutputData,OutLength) then
        begin
          RS232BufferLength:=0;
          SetEvent(SerializationEvent);
          Exit;
        end;
      if (OutLength<=1) then
        Break;
      for j:=i to i+OutLength-HeaderLength-1 do
        RS232Buffer[j]:=OutputData[j-i+HeaderLength];
      i:=i+OutLength-HeaderLength;
    until (RS232BufferLength<=i);
    RS232BufferLength:=i;
    Result:=NO_ERROR;
    SetEvent(SerializationEvent);
  end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232Buffer(var RS232Buffer:array of byte; var RS232BufferLength:integer):integer;  stdcall export;
//;get received bytes from FIFO RS232 buffer
var
  BufferLength,i:integer;
  RS232BufferWriteLocal:integer;
  P:^integer;
begin
  BufferLength:=0;
  Result:=NO_ERROR;
  if RS232BufferLength>0 then
    for i:=1 to RS232BufferLength do
      begin
        P:=@PGlobalRS232Buffer^[GlobalRS232BufferWritePos];
        RS232BufferWriteLocal:=P^;
        if (RS232BufferRead=RS232BufferWriteLocal) then
          Break;
        RS232Buffer[BufferLength]:=PGlobalRS232Buffer^[RS232BufferRead];
        inc(BufferLength);
        RS232BufferRead:=(RS232BufferRead+1) mod (GlobalRS232BufferEnd+1);
      end;
  RS232BufferLength:=BufferLength;
end;
//-------------------------------------------------------------------------------------------------------
function DoRS232BufferSend(var RS232Buffer:array of byte; var RS232BufferLength:integer):integer;  stdcall export;
//;transmit given RS232 buffer to serial line
var
  BufferLength,i:integer;
begin
  BufferLength:=0;
  Result:=NO_ERROR;
  try
    if RS232BufferLength>0 then
      for i:=1 to RS232BufferLength do
        begin
          if DoRS232Send(RS232Buffer[BufferLength])=DEVICE_NOT_PRESENT then
            begin
              Result:=DEVICE_NOT_PRESENT;
              Break;
            end;
          inc(BufferLength);
        end;
  finally
    RS232BufferLength:=BufferLength;
  end;
end;
//-------------------------------------------------------------------------------------------------------
function DoGetRS232BufferThreadProc(Parameter: Pointer): Integer;
//system unique thread for reading device RS232 FIFO into RS232 buffer
var
  Handles:array[0..1] of integer;
  MutexHandles:array[0..1] of integer;
  BufferLength:integer;
  NewRS232BufferWrite:integer;
  i,j:integer;
  P:^integer;
label
  EndOfBufferThreadProc;
begin
  Handles[0]:=RS232BufferGetEvent;
  Handles[1]:=RS232BufferEndEvent;
  MutexHandles[0]:=RS232MutexHandle;
  MutexHandles[1]:=RS232BufferEndEvent;
  Result:=0;
  j:=0;

  LocalThreadRunningEvent:=CreateEvent(nil,false,false,nil); //event to signal end of execution of this thread

  if IsRS232PrevInst then
  case WaitForMultipleObjects(2,@MutexHandles,false,INFINITE) of
    WAIT_OBJECT_0,WAIT_ABANDONED_0:
      begin
        WaitForSingleObject(RS232MutexHandle,3000);
        IsRS232PrevInst:=false;
        P:=@PGlobalRS232Buffer^[GlobalRS232BufferWritePos];
        RS232BufferWrite:=P^;
      end;
    WAIT_OBJECT_0+1,WAIT_ABANDONED_0+1:
      begin
        SetEvent(LocalThreadRunningEvent);
        Exit;
      end;
  end;

  DoGetRS232BufferTimer:=timeSetEvent(RS232BufferTimerInterval,1, TFNTimeCallBack(RS232BufferGetEvent), 0, TIME_PERIODIC or TIME_CALLBACK_EVENT_SET);
  repeat
    case WaitForMultipleObjects(2,@Handles,false,INFINITE) of
      WAIT_OBJECT_0:
        begin
          repeat
            BufferLength:=GlobalRS232BufferEnd-RS232BufferWrite+1;
            if DoGetRS232BufferLocal(PGlobalRS232Buffer^[RS232BufferWrite], BufferLength)<>NO_ERROR then
              begin
                if WaitForSingleObject(RS232BufferEndEvent,2000)<>WAIT_TIMEOUT then // wait 2 seconds if there is no answer (save bandwidth for non-RS232 buffer devices)
                  begin
                    goto EndOfBufferThreadProc;
                  end;
                Break;
              end;
            NewRS232BufferWrite:=RS232BufferWrite+BufferLength;
            j:=Low(TGlobalRS232Buffer);
            if NewRS232BufferWrite>GlobalRS232BufferEnd then
              begin
                for i:=GlobalRS232BufferEnd+1 to NewRS232BufferWrite do
                  begin
                    PGlobalRS232Buffer^[j]:=PGlobalRS232Buffer^[i];
                    inc(j);
                  end;
                NewRS232BufferWrite:=NewRS232BufferWrite mod (GlobalRS232BufferEnd+1);
              end;
            RS232BufferWrite:=NewRS232BufferWrite;
            P:=@PGlobalRS232Buffer^[GlobalRS232BufferWritePos];
            P^:=RS232BufferWrite;
          until j<=Low(TGlobalRS232Buffer);
        end;
      WAIT_OBJECT_0+1:
        begin
            Break;
        end;
    end;
  until false;
EndOfBufferThreadProc:
  if DoGetRS232BufferTimer<>0 then
    begin
      timeKillEvent(DoGetRS232BufferTimer);
      DoGetRS232BufferTimer:=0;
    end;
  SetEvent(LocalThreadRunningEvent);
end;
//-------------------------------------------------------------------------------------------------------
procedure InitRS232ExclusiveSystemBuffer;
//inicialization of system unique RS232 buffer (memory mapped file)
var
  P:^integer;
begin
  RS232MutexHandle:=CreateMutex(nil, false, RS232BufferMutexName);
  if RS232MutexHandle <> 0 then
    begin
      if GetLastError = ERROR_ALREADY_EXISTS then
        IsRS232PrevInst := TRUE
      else
        IsRS232PrevInst := FALSE;
    end
  else
    begin
      IsRS232PrevInst := FALSE;
    end;
  if not IsRS232PrevInst then
    WaitForSingleObject(RS232MutexHandle,3000);
  RS232BufferFileMappedHND:=CreateFileMapping(MAXDWORD,nil,PAGE_READWRITE or SEC_COMMIT	or SEC_NOCACHE,0,GlobalRS232BufferLength,RS232BufferFileMappedName);
  if RS232BufferFileMappedHND=0 then
    Exit;
  PGlobalRS232Buffer:=MapViewOfFile(RS232BufferFileMappedHND,FILE_MAP_WRITE,0,0,GlobalRS232BufferLength);
  if PGlobalRS232Buffer=nil then
    Exit;
  if IsRS232PrevInst then
    begin
      P:=@PGlobalRS232Buffer^[GlobalRS232BufferWritePos];
      RS232BufferRead:=P^;
    end;
end;
//-------------------------------------------------------------------------------------------------------
procedure CloseRS232ExclusiveSystemBuffer;
//close of system unique RS232 buffer (memory mapped file)
begin
  if PGlobalRS232Buffer<>nil then
    begin
      UnmapViewOfFile(PGlobalRS232Buffer);
    end;
  if RS232BufferFileMappedHND>0 then
    begin
      CloseHandle(RS232BufferFileMappedHND);
    end;
  RS232BufferFileMappedHND:=0;
  if not IsRS232PrevInst then
    ReleaseMutex(RS232MutexHandle);
  CloseHandle(RS232MutexHandle);
end;
//-------------------------------------------------------------------------------------------------------
procedure DLLMain(Reason : DWORD);
begin
  case Reason of
    DLL_PROCESS_ATTACH: //next application starts using DLL
      begin
        //All buffers, threads, events and synchronization object create
        InitRS232ExclusiveSystemBuffer; //create RS232 buffer (memory mapped file)
        SerializationEvent:=CreateEvent(nil,false,true,SerializationEventName);// serialization event for device access
        {$IFDEF DemoVersion}
          DemoOK:=MessageBox(0,'Please register!','Free version!',MB_OK or MB_ICONINFORMATION or MB_TASKMODAL or MB_TOPMOST);
        {$ENDIF}
        HookHandle:=SetWindowsHookEx(WH_GETMESSAGE,FNHookProc,0,GetCurrentThreadID); //install Hook function (WM_QUIT message)
        RS232BufferGetEvent:=CreateEvent(nil,false,false,nil); //event for start reading device FIFO
        RS232BufferEndEvent:=CreateEvent(nil,false,false,nil); //event to finish FIFO reading thread
        RS232BufferThreadHND:=BeginThread(nil,0,DoGetRS232BufferThreadProc,nil,CREATE_SUSPENDED,Cardinal(RS232BufferThreadId)); //create FIFO reading thread (suspended)
        SetThreadPriority(RS232BufferThreadHND,THREAD_PRIORITY_TIME_CRITICAL); //highest priority for FIFO reading thread
        ResumeThread(RS232BufferThreadHND); //start FIFO reading thread
      end;
    DLL_PROCESS_DETACH: //next application ends using DLL
      begin
        if HookHandle<>0 then UnhookWindowsHookEx(HookHandle); //uninstall Hook function (WM_QUIT message)
        if RS232BufferThreadHND<>0 then //if Hook function not unistalls thread - then uninstall it
          begin
            if DoGetRS232BufferTimer<>0 then
              begin
                timeKillEvent(DoGetRS232BufferTimer);
                DoGetRS232BufferTimer:=0;
              end;
            SetEvent(RS232BufferEndEvent);
            if LocalThreadRunningEvent<>0 then
              begin
                WaitForSingleObject(LocalThreadRunningEvent,5000);
                WaitForSingleObject(RS232BufferThreadHND,10);
                CloseHandle(RS232BufferEndEvent);
                LocalThreadRunningEvent:=0;
              end
            else
              WaitForSingleObject(RS232BufferThreadHND,400);
            CloseHandle(RS232BufferThreadHND);
          end;
        if LocalThreadRunningEvent<>0 then
          begin
            CloseHandle(RS232BufferEndEvent);
            LocalThreadRunningEvent:=0;
          end;
        CloseHandle(RS232BufferGetEvent); //free event for start reading device FIFO
        CloseHandle(RS232BufferEndEvent); //free event to finish FIFO reading thread
        CloseHandle(SerializationEvent); //free serialization event for device access
        CloseRS232ExclusiveSystemBuffer; //free RS232 buffer (memory mapped file)
        {$IFDEF DemoVersion}
          DemoOK:=MessageBox(0,'Please register!','Free version!',MB_OK or MB_ICONINFORMATION or MB_TASKMODAL or MB_TOPMOST);
        {$ENDIF}
      end;
    DLL_THREAD_ATTACH: //next thread starts using DLL
      begin
        {$IFDEF DemoVersion}
          if Trunc(DemoOK+0.123)<>(abs(OKDemoValue)-1) then Halt; // for bad cracking error
        {$ENDIF}
      end;
    DLL_THREAD_DETACH: //next thread ends using DLL
      begin
      end;
  end;
end;//DLLMain
//-------------------------------------------------------------------------------------------------------
exports
  //exported functions:
  //DoSetInfraBufferEmpty,		//;restart of infra reading (if was stopped by RAM reading)
  DoGetInfraCode,			//;transmit of receved infra code (if some code in infra buffer)
  DoSetDataPortDirection,		//;set direction of data bits
  DoGetDataPortDirection,		//;get direction of data bits
  DoSetOutDataPort,			//;set data bits (if are bits as input, then set theirs pull-ups)
  DoGetOutDataPort,			//;get data bits (if are bits as input, then get theirs pull-ups)
  DoGetInDataPort,			//;get data bits - pin reading
  DoEEPROMRead,				//;read EEPROM from given address
  DoEEPROMWrite,			//;write data byte to EEPROM to given address
  DoRS232Send, 				//;send one data byte to RS232 line
  DoRS232Read,				//;read one data byte from serial line (only when FIFO not implemented in device)
  DoSetRS232Baud,			//;set baud speed of serial line
  DoGetRS232Baud,			//;get baud speed of serial line (exact value)
  DoGetRS232Buffer,                     //;get received bytes from FIFO RS232 buffer
  DoRS232BufferSend,                    //;transmit given RS232 buffer to serial line
  DoSetRS232DataBits,                   //;set number of data bits of serial line
  DoGetRS232DataBits,                   //;get number of data bits of serial line
  DoSetRS232Parity,                     //;set parity of serial line
  DoGetRS232Parity,                     //;get parity of serial line
  DoSetRS232StopBits,                   //;set stopbits of serial line
  DoGetRS232StopBits,                   //;get stopbits of serial line

  DoSetDataPortDirections,		//;set directions of data bits
  DoGetDataPortDirections,		//;get directions of data bits
  DoSetOutDataPorts,			//;set data bits (if are bits as input, then set theirs pull-ups)
  DoGetOutDataPorts,			//;get data bits (if are bits as input, then get theirs pull-ups)
  DoGetInDataPorts;			//;get data bits - pin reading

begin
  DLLProc:=@DllMain;
  DllMain(DLL_PROCESS_ATTACH);
end.

