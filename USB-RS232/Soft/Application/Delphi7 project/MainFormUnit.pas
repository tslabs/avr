unit MainFormUnit;
{
  Author: Ing. Igor Cesko
          http://www.cesko.host.sk
          cesko@internet.sk
  Project: AVR309 application note : USB to UART converter
           www.atmel.com
}

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms,
  StdCtrls, ExtCtrls, Grids, Spin, mmsystem, shellapi, ComCtrls;

const
  WM_USBTIMERMESSAGE = WM_USER + 401;

  NO_ERROR=0;
  DEVICE_NOT_PRESENT=1;
  NO_DATA_AVAILABLE=2;
  INVALID_BAUDRATE=3;
  OVERRUN_ERROR=4;
  INVALID_DATABITS=5;
  INVALID_PARITY=6;
  INVALID_STOPBITS=7;
type
  TMainForm = class(TForm)
    LEDRadioGroup: TRadioGroup;
    EEPROMStringGrid: TStringGrid;
    IRCodeImage: TImage;
    DirectionGroupBox: TGroupBox;
    DataDirectionCheckBox0: TCheckBox;
    DataDirectionCheckBox1: TCheckBox;
    DataDirectionCheckBox2: TCheckBox;
    DataDirectionCheckBox3: TCheckBox;
    DataDirectionCheckBox4: TCheckBox;
    DataDirectionCheckBox5: TCheckBox;
    DataDirectionCheckBox6: TCheckBox;
    DataDirectionCheckBox7: TCheckBox;
    InfraTimer: TTimer;
    OutDataGroupBox: TGroupBox;
    DataOutCheckBox0: TCheckBox;
    DataOutCheckBox1: TCheckBox;
    DataOutCheckBox2: TCheckBox;
    DataOutCheckBox3: TCheckBox;
    DataOutCheckBox4: TCheckBox;
    DataOutCheckBox5: TCheckBox;
    DataOutCheckBox6: TCheckBox;
    DataOutCheckBox7: TCheckBox;
    DataInGroupBox: TGroupBox;
    DataInCheckBox0: TCheckBox;
    DataInCheckBox1: TCheckBox;
    DataInCheckBox2: TCheckBox;
    DataInCheckBox3: TCheckBox;
    DataInCheckBox4: TCheckBox;
    DataInCheckBox5: TCheckBox;
    DataInCheckBox6: TCheckBox;
    DataInCheckBox7: TCheckBox;
    DataInTimer: TTimer;
    InfraIntervalLabel: TLabel;
    RemoteCodeLabel: TLabel;
    DeviceNotPresentLabel: TLabel;
    CopyRightLabel: TLabel;
    RS232BufferTimer: TTimer;
    RS232BufferMemo: TMemo;
    TerminalMemo: TMemo;
    RS232GroupBox: TGroupBox;
    RS232ReadTimer: TTimer;
    RS232BaudLabel: TLabel;
    RS232ReadBaudLabel: TLabel;
    TransmittingLabel: TLabel;
    ParityLabel: TLabel;
    StopBitsLabel: TLabel;
    DataBitsLabel: TLabel;
    ReceivingLabel: TLabel;
    RS232SendButton: TButton;
    RS232BaudrateSpinEdit: TSpinEdit;
    ParityComboBox: TComboBox;
    StopBitsComboBox: TComboBox;
    DataBitsComboBox: TComboBox;
    EEPROMGroupBox: TGroupBox;
    EEPROMReadButton: TButton;
    EEPROMWriteButton: TButton;
    EEPROMSizeSpinEdit: TSpinEdit;
    EEPROMSizeLabel: TLabel;
    PortChoiceRadioGroup: TRadioGroup;
    DataInIntervalLabel: TLabel;
    RS232ReadIntervalLabel: TLabel;
    procedure LEDRadioGroupClick(Sender: TObject);
    procedure InfraTimerTimer(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure DataDirectionCheckBox0Click(Sender: TObject);
    procedure DataOutCheckBox0Click(Sender: TObject);
    procedure DataInTimerTimer(Sender: TObject);
    procedure EEPROMReadButtonClick(Sender: TObject);
    procedure EEPROMWriteButtonClick(Sender: TObject);
    procedure RS232SendButtonClick(Sender: TObject);
    procedure RS232BaudrateSpinEditChange(Sender: TObject);
    procedure RS232ReadTimerTimer(Sender: TObject);
    procedure CopyRightLabelClick(Sender: TObject);
    procedure RS232BufferTimerTimer(Sender: TObject);
    procedure RS232SendEditKeyPress(Sender: TObject; var Key: Char);
    procedure DataBitsComboBoxChange(Sender: TObject);
    procedure ParityComboBoxChange(Sender: TObject);
    procedure StopBitsComboBoxChange(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure EEPROMSizeSpinEditChange(Sender: TObject);
    procedure PortChoiceRadioGroupClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    procedure DrawOsciloscope(var InputData:array of byte; InputLength:integer);
  protected
    procedure WMUSBMeasure(var Message: TMessage); message WM_USBTIMERMESSAGE;
  end;

var
  MainForm: TMainForm;

implementation
{$R *.DFM}

  {$WARN UNSAFE_CODE OFF}
  {$WARN UNSAFE_TYPE OFF}
  {$WARN UNSAFE_CAST OFF}
const
  ClosedApp:boolean=false;
  PreciseTimer:integer=0;
  DLLPath='AVR309.dll';
var
  InputInfraData:array[0..255]of byte;
  InputRS232Data:array[0..10000]of byte;

function DoGetInfraCode(var TimeCodeDiagram:array of byte; var DiagramLength:integer):integer;  stdcall external DLLPath name 'DoGetInfraCode';
function DoSetDataPortDirection(DirectionByte:byte):integer;  stdcall external DLLPath name 'DoSetDataPortDirection';
function DoGetDataPortDirection(var DataDirectionByte:byte):integer;  stdcall external DLLPath name 'DoGetDataPortDirection';
function DoSetOutDataPort(DataOutByte:byte):integer;  stdcall external DLLPath name 'DoSetOutDataPort';
function DoGetOutDataPort(var DataOutByte:byte):integer;  stdcall external DLLPath name 'DoGetOutDataPort';
function DoGetInDataPort(var DataInByte:byte):integer;  stdcall external DLLPath name 'DoGetInDataPort';
function DoEEPROMRead(Address:word; var DataInByte:byte):integer;  stdcall external DLLPath name 'DoEEPROMRead';
function DoEEPROMWrite(Address:word; DataOutByte:byte):integer;  stdcall external DLLPath name 'DoEEPROMWrite';
function DoRS232Send(DataOutByte:byte):integer;  stdcall external DLLPath name 'DoRS232Send';
function DoRS232Read(var DataInByte:byte):integer;  stdcall external DLLPath name 'DoRS232Read';
function DoSetRS232Baud(BaudRate:integer):integer;  stdcall external DLLPath name 'DoSetRS232Baud';
function DoGetRS232Baud(var BaudRate:integer):integer;  stdcall external DLLPath name 'DoGetRS232Baud';
function DoGetRS232Buffer(var RS232Buffer:array of byte; var RS232BufferLength:integer):integer;  stdcall external DLLPath name 'DoGetRS232Buffer';
function DoRS232BufferSend(var RS232Buffer:array of byte; var RS232BufferLength:integer):integer;  stdcall external DLLPath name 'DoRS232BufferSend';

function DoSetRS232DataBits(DataBits:byte):integer;  stdcall external DLLPath name 'DoSetRS232DataBits';
function DoGetRS232DataBits(var DataBits:byte):integer;  stdcall external DLLPath name 'DoGetRS232DataBits';
function DoSetRS232Parity(Parity:byte):integer;  stdcall external DLLPath name 'DoSetRS232Parity';
function DoGetRS232Parity(var Parity:byte):integer;  stdcall external DLLPath name 'DoGetRS232Parity';
function DoSetRS232StopBits(StopBits:byte):integer;  stdcall external DLLPath name 'DoSetRS232StopBits';
function DoGetRS232StopBits(var StopBits:byte):integer;  stdcall external DLLPath name 'DoGetRS232StopBits';

function DoSetDataPortDirections(DirectionByteB, DirectionByteC, DirectionByteD:byte; UsedPorts:byte):integer;  stdcall external DLLPath name 'DoSetDataPortDirections';
function DoGetDataPortDirections(var DataDirectionByteB, DataDirectionByteC, DataDirectionByteD:byte; var UsedPorts:byte):integer;  stdcall external DLLPath name 'DoGetDataPortDirections';
function DoSetOutDataPorts(DataOutByteB, DataOutByteC, DataOutByteD:byte; UsedPorts:byte):integer;  stdcall external DLLPath name 'DoSetOutDataPorts';
function DoGetOutDataPorts(var DataOutByteB, DataOutByteC, DataOutByteD:byte; var UsedPorts:byte):integer;  stdcall external DLLPath name 'DoGetOutDataPorts';
function DoGetInDataPorts(var DataInByteB, DataInByteC, DataInByteD:byte; var UsedPorts:byte):integer;  stdcall external DLLPath name 'DoGetInDataPorts';


procedure TemperatureTimerProc(uID:integer; uMsg:integer;dwUser,dw1,dw2:integer);stdcall;
var
  LocalMainForm:TMainForm;
begin
  LocalMainForm:=TMainForm(dwUser);
  if ClosedApp then Exit;
  PostMessage(LocalMainForm.Handle,WM_USBTIMERMESSAGE,0,0);
end;


procedure TMainForm.WMUSBMeasure(var Message: TMessage);
var
  MyMsg:TMsg;
begin
  Message.Result:=1;
  InfraTimerTimer(self);
  while PeekMessage(MyMsg,Handle,WM_USBTIMERMESSAGE,WM_USBTIMERMESSAGE,PM_REMOVE) do
  begin end; //removing of all waiting WM_USBTIMERMESSAGE messages
  inherited;
end;

procedure TMainForm.LEDRadioGroupClick(Sender: TObject);
var
  PortValue:byte;
begin
  PortValue:=1 shl (LEDRadioGroup.ItemIndex);
  DoSetOutDataPorts(PortValue,PortValue,PortValue,1 shl PortChoiceRadioGroup.ItemIndex);
  DataInTimerTimer(self);
end;

procedure TMainForm.InfraTimerTimer(Sender: TObject);
var
  DataLength:integer;
begin
  InfraIntervalLabel.Caption:=IntToStr(InfraTimer.Interval)+'ms';
  if (DoGetInfraCode(InputInfraData,DataLength)<>NO_ERROR) then
    begin
      //DeviceNotPresentLabel.Visible:=true;
      Exit;
    end;
  DeviceNotPresentLabel.Visible:=false;
  if (DataLength=0) then Exit;
  DrawOsciloscope(InputInfraData,DataLength);
end;

procedure TMainForm.DrawOsciloscope(var InputData:array of byte; InputLength:integer);
var
  i:integer;
  x,y,yHi,yLo:integer;
  TotalLength:integer;
  OscRect:TRect;
const
  vyska=50;
  Xzaciatok=10;
  Xkoniec=10;
begin
  TotalLength:=Xzaciatok+Xkoniec;
  for i:=0 to InputLength-1 do
    TotalLength:=TotalLength+InputData[i];

  IRCodeImage.Picture.Bitmap.Width:=TotalLength;
  IRCodeImage.Picture.Bitmap.Height:=IRCodeImage.Height;
  IRCodeImage.Picture.Bitmap.Canvas.Brush.Color:=clBlack;
  IRCodeImage.Picture.Bitmap.Canvas.Brush.Style:=bsSolid;
  IRCodeImage.Picture.Bitmap.Canvas.Pen.Color:=clYellow;
  IRCodeImage.Picture.Bitmap.Canvas.Pen.Style:=psSolid;
  IRCodeImage.Picture.Bitmap.Canvas.Pen.Mode:=pmCopy;

  OscRect.Top:=0;
  OscRect.Left:=0;
  OscRect.Right:=IRCodeImage.Picture.Bitmap.Width;
  OscRect.Bottom:=IRCodeImage.Picture.Bitmap.Height;
  IRCodeImage.Picture.Bitmap.Canvas.FillRect(OscRect);
  yHi:=IRCodeImage.Picture.Bitmap.Height-vyska;
  yLo:=IRCodeImage.Picture.Bitmap.Height-5;
  x:=IRCodeImage.Picture.Bitmap.Width;
  y:=yHi;
  IRCodeImage.Picture.Bitmap.Canvas.MoveTo(x,y);
  x:=x-Xkoniec;
  IRCodeImage.Picture.Bitmap.Canvas.LineTo(x,y);
  y:=yLo;
  IRCodeImage.Picture.Bitmap.Canvas.LineTo(x,y);

  for i:=InputLength-1 downto 0 do
    begin
      x:=x-InputData[i];
      IRCodeImage.Picture.Bitmap.Canvas.LineTo(x,y);
      if y=yLo then y:=yHi
      else y:=yLo;
      IRCodeImage.Picture.Bitmap.Canvas.LineTo(x,y);
    end;
  IRCodeImage.Picture.Bitmap.Canvas.LineTo(x-Xzaciatok,y);
  IRCodeImage.Repaint;
end;


procedure TMainForm.FormCreate(Sender: TObject);
begin
  Application.HintColor:=clYellow;
  Application.HintPause:=0;
  Application.HintHidePause:=20000;
  //************ Uncomment this next 3 lines if you want higher speed in IR code polling *******************
  //PreciseTimer:=timeSetEvent(InfraTimer.Interval,1, TemperatureTimerProc, integer(self), TIME_PERIODIC);
  //InfraTimer.Enabled:=false;
  //SetThreadPriority(GetCurrentThread,THREAD_PRIORITY_TIME_CRITICAL);
  //********************************************************************************************************
  RS232BaudrateSpinEditChange(self);
  DataBitsComboBox.ItemIndex:=3;
  ParityComboBox.ItemIndex:=0;
  StopBitsComboBox.ItemIndex:=0;
  DataBitsComboBoxChange(self);
  ParityComboBoxChange(self);
  StopBitsComboBoxChange(self);
  EEPROMSizeSpinEditChange(self);

  DataInIntervalLabel.Parent:=PortChoiceRadioGroup;
  DataInIntervalLabel.Left:=PortChoiceRadioGroup.Width-DataInIntervalLabel.Width-5;
  DataInIntervalLabel.Top:=PortChoiceRadioGroup.Height-DataInIntervalLabel.Height-5;
end;

procedure TMainForm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  ClosedApp:=true;
  if PreciseTimer<>0 then
    begin
      timeKillEvent(PreciseTimer);
      PreciseTimer:=0;
    end;
end;

procedure TMainForm.DataDirectionCheckBox0Click(Sender: TObject);
var
  DirectionByte:byte;
  i:integer;
  MyComponent:TComponent;
begin
  if DataInTimer.Tag<>0 then Exit;
  DirectionByte:=0;
  for i:= 0 to 7 do
    begin
      MyComponent:=FindComponent('DataDirectionCheckBox'+IntToStr(i));
      if (MyComponent as TCheckBox).Checked then
        DirectionByte:=DirectionByte+MyComponent.Tag;
    end;
  DoSetDataPortDirections(DirectionByte,DirectionByte,DirectionByte,1 shl PortChoiceRadioGroup.ItemIndex);
  DataInTimerTimer(self);
end;


procedure TMainForm.DataOutCheckBox0Click(Sender: TObject);
var
  OutByte:byte;
  i:integer;
  MyComponent:TComponent;
begin
  if DataInTimer.Tag<>0 then Exit;
  OutByte:=0;
  for i:= 0 to 7 do
    begin
      MyComponent:=FindComponent('DataOutCheckBox'+IntToStr(i));
      if (MyComponent as TCheckBox).Checked then
        OutByte:=OutByte+MyComponent.Tag;
    end;
  DoSetOutDataPorts(OutByte,OutByte,OutByte,1 shl PortChoiceRadioGroup.ItemIndex);
  DataInTimerTimer(self);
end;


procedure TMainForm.DataInTimerTimer(Sender: TObject);
var
  InByte,InByteB,InByteC,InByteD:byte;
  UsedPorts:byte;
  i:integer;
  MyComponent:TComponent;
begin
  DataInTimer.Tag:=1;
  DataInIntervalLabel.Caption:=IntToStr(DataInTimer.Interval)+'ms';
  if DoGetInDataPorts(InByteB,InByteC,InByteD,UsedPorts)<>NO_ERROR then
    begin
      DeviceNotPresentLabel.Visible:=true;
      DataInTimer.Tag:=0;
      Exit;
    end;
  if UsedPorts<=1 then PortChoiceRadioGroup.ItemIndex:=0;
  case PortChoiceRadioGroup.ItemIndex of
    1:InByte:=InByteC;
    2:InByte:=InByteD;
  else
    InByte:=InByteB;
  end;
  DeviceNotPresentLabel.Visible:=false;
  for i:= 0 to 7 do
    begin
      MyComponent:=FindComponent('DataInCheckBox'+IntToStr(i));
      (MyComponent as TCheckBox).Checked:=((MyComponent as TCheckBox).Tag and InByte)<>0;
    end;

  if DoGetOutDataPorts(InByteB,InByteC,InByteD,UsedPorts)<>NO_ERROR then
    begin
      DeviceNotPresentLabel.Visible:=true;
      DataInTimer.Tag:=0;
      Exit;
    end;
  if UsedPorts<=1 then PortChoiceRadioGroup.ItemIndex:=0;
  case PortChoiceRadioGroup.ItemIndex of
    1:InByte:=InByteC;
    2:InByte:=InByteD;
  else
    InByte:=InByteB;
  end;
  DeviceNotPresentLabel.Visible:=false;
  for i:= 0 to 7 do
    begin
      MyComponent:=FindComponent('DataOutCheckBox'+IntToStr(i));
      (MyComponent as TCheckBox).Checked:=((MyComponent as TCheckBox).Tag and InByte)<>0;
    end;

  if DoGetDataPortDirections(InByteB,InByteC,InByteD,UsedPorts)<>NO_ERROR then
    begin
      DeviceNotPresentLabel.Visible:=true;
      DataInTimer.Tag:=0;
      Exit;
    end;
  if UsedPorts<=1 then PortChoiceRadioGroup.ItemIndex:=0;
  case PortChoiceRadioGroup.ItemIndex of
    1:InByte:=InByteC;
    2:InByte:=InByteD;
  else
    InByte:=InByteB;
  end;
  DeviceNotPresentLabel.Visible:=false;
  for i:= 0 to 7 do
    begin
      MyComponent:=FindComponent('DataDirectionCheckBox'+IntToStr(i));
      (MyComponent as TCheckBox).Checked:=((MyComponent as TCheckBox).Tag and InByte)<>0;
    end;
  DataInTimer.Tag:=0;
end;


procedure TMainForm.EEPROMReadButtonClick(Sender: TObject);
var
  i:integer;
  DataByte:byte;
begin
  EEPROMStringGrid.Col:=1;
  for i:=0 to EEPROMStringGrid.RowCount-1 do
    begin
      if DoEEPROMRead(i,DataByte)=0 then
        begin
          EEPROMStringGrid.Cells[1,i]:=IntToStr(DataByte);
          EEPROMStringGrid.Row:=i;
        end
      else
        begin
          raise Exception.Create('Unable to read from EEPROM!'+#13+#10+'(maybe device is not present)');
        end;
    end;
end;

procedure TMainForm.EEPROMWriteButtonClick(Sender: TObject);
var
  i:integer;
  DataByte:integer;
begin
  EEPROMStringGrid.Col:=1;
  try
    for i:=0 to EEPROMStringGrid.RowCount-1 do
      begin
        EEPROMStringGrid.Row:=i;
        DataByte:=StrToInt(EEPROMStringGrid.Cells[1,i]);
        if (DataByte>255)or(DataByte<0) then
          raise Exception.Create('Number is not in BYTE range: 0-255 !');
        if DoEEPROMWrite(i,DataByte)<>0 then
          begin
            raise Exception.Create('Error to write to EEPROM!'+#13+#10+'(maybe device is not present)');
          end;
      end;
  except on EConvertError do
    begin
      ActiveControl:=EEPROMStringGrid;
      MessageBox(Handle,'Invalid BYTE number'+#13+#10+'(unable to convert to numeric value)',PChar(Caption),MB_ICONERROR);
    end
  else
    begin
      ActiveControl:=EEPROMStringGrid;
      raise;
    end;
  end;
end;


procedure TMainForm.RS232SendButtonClick(Sender: TObject);
type
  MyBuffer=array [0..100000] of byte;
var
  j,i:integer;
  P:MyBuffer;
  MyStr:string;
begin
  MyStr:=TerminalMemo.Text;
  i:=Length(MyStr);
  Screen.Cursor:=crHourGlass;
  TransmittingLabel.Visible:=true;
  TransmittingLabel.Repaint;
  try
    for j:=1 to i do
      begin
        P[j-1]:=byte(MyStr[j]);
      end;
    DoRS232BufferSend(P, i);
  finally
    Screen.Cursor:=crArrow;
    TransmittingLabel.Visible:=false;
    TransmittingLabel.Repaint;
  end;
end;


procedure TMainForm.RS232BaudrateSpinEditChange(Sender: TObject);
var
  RetVal:integer;
begin
  try
    RetVal:=DoSetRS232Baud(RS232BaudrateSpinEdit.Value);
    RS232BaudrateSpinEdit.Color:=clWindow;
    RS232BaudrateSpinEdit.Font.Style:=[];
    case RetVal of
      DEVICE_NOT_PRESENT:RS232BaudrateSpinEdit.Color:=clRed;
      INVALID_BAUDRATE: RS232BaudrateSpinEdit.Font.Style:=[fsStrikeOut];
    end;
  except
  end;
end;

procedure TMainForm.RS232ReadTimerTimer(Sender: TObject);
var
  BaudRate:integer;
  Databits:byte;
  Parity:byte;
  Stopbits:byte;
begin
  RS232ReadIntervalLabel.Caption:=IntToStr(RS232ReadTimer.Interval)+'ms';
  if DoGetRS232Baud(BaudRate)=NO_ERROR then
    begin
      RS232ReadBaudLabel.Caption:='='+IntToStr(BaudRate);
      DeviceNotPresentLabel.Visible:=false;
    end
  else
    begin
      DeviceNotPresentLabel.Visible:=true;
    end;
  if not DataBitsComboBox.DroppedDown then
    if DoGetRS232DataBits(Databits)=NO_ERROR then
      begin
        DataBitsComboBox.ItemIndex:=Databits-5;
        DeviceNotPresentLabel.Visible:=false;
      end
    else
      begin
        //DeviceNotPresentLabel.Visible:=true;
      end;
  if not ParityComboBox.DroppedDown then
    if DoGetRS232Parity(Parity)=NO_ERROR then
      begin
        ParityComboBox.ItemIndex:=Parity;
        DeviceNotPresentLabel.Visible:=false;
      end
    else
      begin
        //DeviceNotPresentLabel.Visible:=true;
      end;
  if not StopBitsComboBox.DroppedDown then
    if DoGetRS232StopBits(Stopbits)=NO_ERROR then
      begin
        StopBitsComboBox.ItemIndex:=Stopbits;
        DeviceNotPresentLabel.Visible:=false;
      end
    else
      begin
        //DeviceNotPresentLabel.Visible:=true;
      end;
end;

procedure TMainForm.CopyRightLabelClick(Sender: TObject);
begin
  ShellExecute(Handle,'open','http://www.cesko.host.sk',nil,'.',0);
  ShellExecute(Handle,'open','http://www.atmel.com',nil,'.',0);
end;

procedure TMainForm.RS232BufferTimerTimer(Sender: TObject);
var
  DataLength:integer;
  i:integer;
begin
  DataLength:=SizeOf(InputRS232Data);
  if (DoGetRS232Buffer(InputRS232Data,DataLength)<>NO_ERROR) then
    begin
      //DeviceNotPresentLabel.Visible:=true;
      Exit;
    end;
  if (DataLength=0) then Exit;
  Screen.Cursor:=crAppStart;
  ReceivingLabel.Visible:=true;
  ReceivingLabel.Repaint;
  try
    TerminalMemo.SelStart:=Length(TerminalMemo.Text);
    TerminalMemo.SelLength:=0;
    for i:=0 to DataLength-1 do
      begin
        RS232BufferMemo.Lines.Add(IntToStr(InputRS232Data[i])+' -> '+IntToHex(InputRS232Data[i],2)+' -> '+chr(InputRS232Data[i]));
        TerminalMemo.SelText:=chr(InputRS232Data[i]);
      end;
  finally
    Screen.Cursor:=crArrow;
    ReceivingLabel.Visible:=false;
    ReceivingLabel.Repaint;
  end;
end;

procedure TMainForm.RS232SendEditKeyPress(Sender: TObject; var Key: Char);
begin
  DoRS232Send(byte(Key));
end;

procedure TMainForm.DataBitsComboBoxChange(Sender: TObject);
var
  RetVal:integer;
begin
  RetVal:=DoSetRS232DataBits(StrToInt(DataBitsComboBox.Text));
  DataBitsComboBox.Color:=clWindow;
  DataBitsComboBox.Font.Style:=[];
  case RetVal of
    DEVICE_NOT_PRESENT: DataBitsComboBox.Color:=clRed;
    INVALID_DATABITS: DataBitsComboBox.Font.Style:=[fsStrikeOut];
  end;
end;

procedure TMainForm.ParityComboBoxChange(Sender: TObject);
var
  RetVal:integer;
begin
  RetVal:=DoSetRS232Parity(ParityComboBox.ItemIndex);
  ParityComboBox.Color:=clWindow;
  ParityComboBox.Font.Style:=[];
  case RetVal of
    DEVICE_NOT_PRESENT: ParityComboBox.Color:=clRed;
    INVALID_PARITY: ParityComboBox.Font.Style:=[fsStrikeOut];
  end;
end;

procedure TMainForm.StopBitsComboBoxChange(Sender: TObject);
var
  RetVal:integer;
begin
  RetVal:=DoSetRS232StopBits(StopBitsComboBox.ItemIndex);
  StopBitsComboBox.Color:=clWindow;
  StopBitsComboBox.Font.Style:=[];
  case RetVal of
    DEVICE_NOT_PRESENT: StopBitsComboBox.Color:=clRed;
    INVALID_STOPBITS: StopBitsComboBox.Font.Style:=[fsStrikeOut];
  end;
end;


procedure TMainForm.FormResize(Sender: TObject);
begin
  TerminalMemo.Width:= ClientWidth-EEPROMStringGrid.Width-TerminalMemo.Left;
  EEPROMStringGrid.Left:=ClientWidth-EEPROMStringGrid.Width;
  TerminalMemo.Width:= EEPROMStringGrid.Left-TerminalMemo.Left;
  TerminalMemo.Height:= ClientHeight-TerminalMemo.Top;
  RS232BufferMemo.Height:= ClientHeight-RS232BufferMemo.Top;
end;





procedure TMainForm.EEPROMSizeSpinEditChange(Sender: TObject);
var
  i:integer;
begin
  EEPROMStringGrid.RowCount:=EEPROMSizeSpinEdit.Value;
  for i:=0 to EEPROMStringGrid.RowCount-1 do
    EEPROMStringGrid.Cells[0,i]:=IntToStr(i);  
end;


procedure TMainForm.PortChoiceRadioGroupClick(Sender: TObject);
begin
  DataInTimerTimer(self);
end;


end.
