object MainForm: TMainForm
  Left = 296
  Top = 38
  Width = 512
  Height = 688
  Caption = 'AVR to USB tester - AVR309'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = True
  Position = poDefaultPosOnly
  OnClose = FormClose
  OnCreate = FormCreate
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object IRCodeImage: TImage
    Left = 1
    Top = 322
    Width = 416
    Height = 59
    Hint = 'Oscilograph of received IR code'
    AutoSize = True
    ParentShowHint = False
    ShowHint = True
  end
  object InfraIntervalLabel: TLabel
    Left = 2
    Top = 324
    Width = 25
    Height = 13
    Hint = 'Refresh time for IR code question'
    Caption = '50ms'
    Color = clWhite
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
  end
  object RemoteCodeLabel: TLabel
    Left = 8
    Top = 309
    Width = 139
    Height = 13
    Caption = 'IR Remote Code oscilograph:'
  end
  object DeviceNotPresentLabel: TLabel
    Left = 251
    Top = 224
    Width = 67
    Height = 26
    Hint = 'If device is not present, you see this warning'
    Alignment = taCenter
    Caption = 'Device not present !!!'
    Color = clRed
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentColor = False
    ParentFont = False
    ParentShowHint = False
    ShowHint = True
    Visible = False
    WordWrap = True
  end
  object CopyRightLabel: TLabel
    Left = 288
    Top = 296
    Width = 124
    Height = 17
    Cursor = crHandPoint
    Hint = 'For more information see my web page'
    Caption = 'Atmel and authors website'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlue
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsUnderline]
    ParentFont = False
    ParentShowHint = False
    ShowHint = True
    OnClick = CopyRightLabelClick
  end
  object DataInIntervalLabel: TLabel
    Left = 288
    Top = 184
    Width = 31
    Height = 13
    Hint = 'Refresh time for data port polling'
    Caption = '200ms'
    Color = clWhite
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
  end
  object PortChoiceRadioGroup: TRadioGroup
    Left = 8
    Top = 154
    Width = 409
    Height = 33
    Hint = 'Select AVR port for read/write'
    Caption = 'Select data port:'
    Color = clBtnFace
    Columns = 4
    ItemIndex = 0
    Items.Strings = (
      'port B (or base)'
      'port C'
      'port D')
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 9
    OnClick = PortChoiceRadioGroupClick
  end
  object OutDataGroupBox: TGroupBox
    Left = 8
    Top = 4
    Width = 113
    Height = 145
    Hint = 
      'if Direction=out -> output data signal, if Direction=in -> enabl' +
      'e pull-up on data pin'
    Caption = 'Data port Out/pullup:'
    Color = clBtnFace
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 3
    object DataOutCheckBox0: TCheckBox
      Tag = 1
      Left = 16
      Top = 16
      Width = 55
      Height = 17
      Caption = 'Out0'
      Checked = True
      State = cbChecked
      TabOrder = 0
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox1: TCheckBox
      Tag = 2
      Left = 16
      Top = 31
      Width = 55
      Height = 17
      Caption = 'Out1'
      Checked = True
      State = cbChecked
      TabOrder = 1
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox2: TCheckBox
      Tag = 4
      Left = 16
      Top = 48
      Width = 55
      Height = 15
      Caption = 'Out2'
      Checked = True
      State = cbChecked
      TabOrder = 2
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox3: TCheckBox
      Tag = 8
      Left = 16
      Top = 61
      Width = 55
      Height = 17
      Caption = 'Out3'
      Checked = True
      State = cbChecked
      TabOrder = 3
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox4: TCheckBox
      Tag = 16
      Left = 16
      Top = 75
      Width = 55
      Height = 17
      Caption = 'Out4'
      Checked = True
      State = cbChecked
      TabOrder = 4
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox5: TCheckBox
      Tag = 32
      Left = 16
      Top = 90
      Width = 55
      Height = 17
      Caption = 'Out5'
      Checked = True
      State = cbChecked
      TabOrder = 5
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox6: TCheckBox
      Tag = 64
      Left = 16
      Top = 105
      Width = 55
      Height = 17
      Caption = 'Out6'
      Checked = True
      State = cbChecked
      TabOrder = 6
      OnClick = DataOutCheckBox0Click
    end
    object DataOutCheckBox7: TCheckBox
      Tag = 128
      Left = 16
      Top = 120
      Width = 55
      Height = 17
      Caption = 'Out7'
      Checked = True
      State = cbChecked
      TabOrder = 7
      OnClick = DataOutCheckBox0Click
    end
  end
  object LEDRadioGroup: TRadioGroup
    Left = 328
    Top = 4
    Width = 89
    Height = 145
    Hint = 'switch ON selected LED (connected to data bus: cathode to GND)'
    Caption = 'LED ON/OFF:'
    Color = clBtnFace
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Items.Strings = (
      'LED 0'
      'LED 1'
      'LED 2'
      'LED 3'
      'LED 4'
      'LED 5'
      'LED 6'
      'LED 7')
    ParentColor = False
    ParentFont = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 0
    OnClick = LEDRadioGroupClick
  end
  object EEPROMStringGrid: TStringGrid
    Left = 422
    Top = 0
    Width = 82
    Height = 661
    Hint = 'Data for EEPROM (read or write)'
    Align = alRight
    ColCount = 2
    DefaultColWidth = 20
    DefaultRowHeight = 12
    RowCount = 1024
    FixedRows = 0
    Font.Charset = EASTEUROPE_CHARSET
    Font.Color = clWindowText
    Font.Height = -9
    Font.Name = 'Arial'
    Font.Style = []
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing, goEditing, goThumbTracking]
    ParentFont = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 1
    ColWidths = (
      20
      40)
  end
  object DirectionGroupBox: TGroupBox
    Left = 128
    Top = 4
    Width = 105
    Height = 145
    Hint = 'Direction of data bus (out=checked, in=unchecked)'
    Caption = 'Data port Direction:'
    Color = clBtnFace
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 2
    object DataDirectionCheckBox0: TCheckBox
      Tag = 1
      Left = 16
      Top = 16
      Width = 80
      Height = 17
      Caption = 'Direction0'
      TabOrder = 0
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox1: TCheckBox
      Tag = 2
      Left = 16
      Top = 31
      Width = 80
      Height = 17
      Caption = 'Direction1'
      TabOrder = 1
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox2: TCheckBox
      Tag = 4
      Left = 16
      Top = 46
      Width = 80
      Height = 17
      Caption = 'Direction2'
      TabOrder = 2
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox3: TCheckBox
      Tag = 8
      Left = 16
      Top = 61
      Width = 80
      Height = 17
      Caption = 'Direction3'
      TabOrder = 3
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox4: TCheckBox
      Tag = 16
      Left = 16
      Top = 75
      Width = 80
      Height = 17
      Caption = 'Direction4'
      TabOrder = 4
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox5: TCheckBox
      Tag = 32
      Left = 16
      Top = 90
      Width = 80
      Height = 17
      Caption = 'Direction5'
      ParentShowHint = False
      ShowHint = False
      TabOrder = 5
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox6: TCheckBox
      Tag = 64
      Left = 16
      Top = 105
      Width = 80
      Height = 17
      Caption = 'Direction6'
      TabOrder = 6
      OnClick = DataDirectionCheckBox0Click
    end
    object DataDirectionCheckBox7: TCheckBox
      Tag = 128
      Left = 16
      Top = 120
      Width = 80
      Height = 17
      Caption = 'Direction7'
      TabOrder = 7
      OnClick = DataDirectionCheckBox0Click
    end
  end
  object DataInGroupBox: TGroupBox
    Left = 240
    Top = 4
    Width = 73
    Height = 145
    Hint = 'State of data pins'
    Caption = 'Data port In:'
    Color = clBtnFace
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 4
    object DataInCheckBox0: TCheckBox
      Tag = 1
      Left = 16
      Top = 16
      Width = 45
      Height = 17
      Caption = 'In0'
      TabOrder = 0
    end
    object DataInCheckBox1: TCheckBox
      Tag = 2
      Left = 16
      Top = 31
      Width = 45
      Height = 17
      Caption = 'In1'
      TabOrder = 1
    end
    object DataInCheckBox2: TCheckBox
      Tag = 4
      Left = 16
      Top = 46
      Width = 45
      Height = 17
      Caption = 'In2'
      TabOrder = 2
    end
    object DataInCheckBox3: TCheckBox
      Tag = 8
      Left = 16
      Top = 61
      Width = 45
      Height = 17
      Caption = 'In3'
      TabOrder = 3
    end
    object DataInCheckBox4: TCheckBox
      Tag = 16
      Left = 16
      Top = 75
      Width = 45
      Height = 17
      Caption = 'In4'
      TabOrder = 4
    end
    object DataInCheckBox5: TCheckBox
      Tag = 32
      Left = 16
      Top = 90
      Width = 45
      Height = 17
      Caption = 'In5'
      TabOrder = 5
    end
    object DataInCheckBox6: TCheckBox
      Tag = 64
      Left = 16
      Top = 105
      Width = 45
      Height = 17
      Caption = 'In6'
      TabOrder = 6
    end
    object DataInCheckBox7: TCheckBox
      Tag = 128
      Left = 16
      Top = 120
      Width = 45
      Height = 17
      Caption = 'In7'
      TabOrder = 7
    end
  end
  object RS232BufferMemo: TMemo
    Left = 0
    Top = 384
    Width = 121
    Height = 274
    Hint = 'Received data: DEC->HEX->ASCII'
    Lines.Strings = (
      'DEC->HEX->ASCII')
    ParentShowHint = False
    ScrollBars = ssVertical
    ShowHint = True
    TabOrder = 5
    WordWrap = False
  end
  object TerminalMemo: TMemo
    Left = 128
    Top = 384
    Width = 289
    Height = 273
    Hint = 'Text to transmit/Received text'
    Lines.Strings = (
      'Text to transmit/Received text')
    ParentShowHint = False
    ScrollBars = ssBoth
    ShowHint = True
    TabOrder = 6
    WantTabs = True
  end
  object RS232GroupBox: TGroupBox
    Left = 8
    Top = 192
    Width = 241
    Height = 113
    Hint = 'RS232 settings'
    Caption = 'RS232:'
    Color = clBtnFace
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 7
    object RS232BaudLabel: TLabel
      Left = 8
      Top = 21
      Width = 28
      Height = 13
      Caption = 'Baud:'
    end
    object RS232ReadBaudLabel: TLabel
      Left = 106
      Top = 19
      Width = 56
      Height = 13
      Hint = 'exact setted baudrate'
      Caption = '(True Baud)'
      ParentShowHint = False
      ShowHint = True
    end
    object TransmittingLabel: TLabel
      Left = 168
      Top = 48
      Width = 66
      Height = 13
      Caption = 'Transmitting...'
      Color = clYellow
      ParentColor = False
      Visible = False
    end
    object ParityLabel: TLabel
      Left = 8
      Top = 69
      Width = 29
      Height = 13
      Caption = 'Parity:'
    end
    object StopBitsLabel: TLabel
      Left = 8
      Top = 93
      Width = 42
      Height = 13
      Caption = 'StopBits:'
    end
    object DataBitsLabel: TLabel
      Left = 8
      Top = 46
      Width = 43
      Height = 13
      Caption = 'DataBits:'
    end
    object ReceivingLabel: TLabel
      Left = 168
      Top = 64
      Width = 57
      Height = 13
      Caption = 'Receiving...'
      Color = clFuchsia
      ParentColor = False
      Visible = False
    end
    object RS232ReadIntervalLabel: TLabel
      Left = 202
      Top = 92
      Width = 31
      Height = 13
      Hint = 'Refresh time for serial parameters polling'
      Caption = '200ms'
      Color = clWhite
      ParentColor = False
      ParentShowHint = False
      ShowHint = True
    end
    object RS232SendButton: TButton
      Left = 166
      Top = 16
      Width = 67
      Height = 25
      Hint = 'Send text to device RS232'
      Caption = 'RS232 send'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
      OnClick = RS232SendButtonClick
    end
    object RS232BaudrateSpinEdit: TSpinEdit
      Left = 37
      Top = 16
      Width = 65
      Height = 22
      Hint = 'Set baudrate on device RS232'
      Increment = 100
      MaxLength = 7
      MaxValue = 1500000
      MinValue = 300
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      Value = 57600
      OnChange = RS232BaudrateSpinEditChange
    end
    object ParityComboBox: TComboBox
      Left = 48
      Top = 64
      Width = 57
      Height = 21
      Hint = 'Set parity on device RS232'
      Style = csDropDownList
      ItemHeight = 13
      ParentShowHint = False
      ShowHint = True
      TabOrder = 2
      OnChange = ParityComboBoxChange
      Items.Strings = (
        'none'
        'odd'
        'even'
        'mark'
        'space')
    end
    object StopBitsComboBox: TComboBox
      Left = 56
      Top = 88
      Width = 49
      Height = 21
      Hint = 'Set stopbits on device RS232'
      Style = csDropDownList
      ItemHeight = 13
      ParentShowHint = False
      ShowHint = True
      TabOrder = 3
      OnChange = StopBitsComboBoxChange
      Items.Strings = (
        '1'
        '2')
    end
    object DataBitsComboBox: TComboBox
      Left = 56
      Top = 41
      Width = 49
      Height = 21
      Hint = 'Set databits on device RS232'
      Style = csDropDownList
      ItemHeight = 13
      ParentShowHint = False
      ShowHint = True
      TabOrder = 4
      OnChange = DataBitsComboBoxChange
      Items.Strings = (
        '5'
        '6'
        '7'
        '8')
    end
  end
  object EEPROMGroupBox: TGroupBox
    Left = 320
    Top = 192
    Width = 97
    Height = 100
    Hint = 'Data EEPROM'
    Caption = 'EEPROM:'
    Color = clBtnFace
    ParentColor = False
    ParentShowHint = False
    ShowHint = True
    TabOrder = 8
    object EEPROMSizeLabel: TLabel
      Left = 12
      Top = 77
      Width = 21
      Height = 13
      Caption = 'size:'
    end
    object EEPROMReadButton: TButton
      Left = 8
      Top = 16
      Width = 81
      Height = 25
      Hint = 'Read full EEPROM to cells'
      Caption = 'EEPROM read'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 0
      OnClick = EEPROMReadButtonClick
    end
    object EEPROMWriteButton: TButton
      Left = 8
      Top = 44
      Width = 81
      Height = 25
      Hint = 'Write all cells to EEPROM'
      Caption = 'EEPROM write'
      ParentShowHint = False
      ShowHint = True
      TabOrder = 1
      OnClick = EEPROMWriteButtonClick
    end
    object EEPROMSizeSpinEdit: TSpinEdit
      Left = 40
      Top = 72
      Width = 49
      Height = 22
      Hint = 'EEPROM size (AT90S23x3=128, ATmega8=512)'
      Increment = 128
      MaxLength = 4
      MaxValue = 9999
      MinValue = 0
      ParentShowHint = False
      ShowHint = True
      TabOrder = 2
      Value = 128
      OnChange = EEPROMSizeSpinEditChange
    end
  end
  object InfraTimer: TTimer
    Interval = 50
    OnTimer = InfraTimerTimer
    Left = 384
    Top = 328
  end
  object DataInTimer: TTimer
    Interval = 200
    OnTimer = DataInTimerTimer
    Left = 296
    Top = 120
  end
  object RS232BufferTimer: TTimer
    Interval = 100
    OnTimer = RS232BufferTimerTimer
    Left = 168
    Top = 424
  end
  object RS232ReadTimer: TTimer
    Interval = 500
    OnTimer = RS232ReadTimerTimer
    Left = 128
    Top = 232
  end
end
