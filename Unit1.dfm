object Form1: TForm1
  Left = 563
  Top = 393
  Anchors = []
  Caption = 'CSVgraph'
  ClientHeight = 342
  ClientWidth = 617
  Color = clBtnFace
  DragMode = dmAutomatic
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Visible = True
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  DesignSize = (
    617
    342)
  PixelsPerInch = 96
  TextHeight = 13
  object Results: TRichEdit
    Left = 0
    Top = 0
    Width = 617
    Height = 343
    Anchors = [akLeft, akTop, akRight, akBottom]
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Lines.Strings = (
      '')
    ParentFont = False
    ReadOnly = True
    ScrollBars = ssBoth
    TabOrder = 0
    WordWrap = False
    Zoom = 100
  end
end
