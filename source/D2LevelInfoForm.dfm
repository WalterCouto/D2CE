object LevelInfoForm: TLevelInfoForm
  Left = 389
  Top = 103
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Level Requirements'
  ClientHeight = 430
  ClientWidth = 392
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  PixelsPerInch = 96
  TextHeight = 13
  object LevelInfoGrid: TStringGrid
    Left = 0
    Top = 0
    Width = 392
    Height = 340
    Align = alTop
    ColCount = 4
    DefaultColWidth = 100
    DefaultRowHeight = 20
    RowCount = 100
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goThumbTracking]
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 0
    RowHeights = (
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20
      20)
  end
  object CloseButton: TButton
    Left = 0
    Top = 384
    Width = 392
    Height = 45
    Caption = '&CLOSE'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 1
    OnClick = CloseButtonClick
  end
  object D2Version: TRadioGroup
    Left = 95
    Top = 344
    Width = 202
    Height = 36
    Caption = 'Diablo II Version'
    Columns = 2
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ItemIndex = 0
    Items.Strings = (
      'Pre-1.07'
      '1.07+')
    ParentFont = False
    TabOrder = 2
    OnClick = D2VersionClick
  end
end
