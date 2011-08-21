program AVR309USBdemo;
{
  Author: Ing. Igor Cesko
          http://www.cesko.host.sk
          cesko@internet.sk
  Project: AVR309 application note : USB to UART converter
           www.atmel.com
}

uses
  Forms,
  MainFormUnit in 'MainFormUnit.pas' {MainForm};

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'AVR309 test application';
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.
