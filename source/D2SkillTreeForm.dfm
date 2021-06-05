object SkillTrees: TSkillTrees
  Left = 256
  Top = 104
  ActiveControl = ViewOption
  BorderIcons = []
  BorderStyle = bsDialog
  Caption = 'Character Skills'
  ClientHeight = 411
  ClientWidth = 232
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnShow = FormShow
  DesignSize = (
    232
    411)
  PixelsPerInch = 96
  TextHeight = 16
  object SkillsGrid: TStringGrid
    Left = 6
    Top = 6
    Width = 219
    Height = 256
    Anchors = [akTop]
    ColCount = 2
    DefaultColWidth = 138
    DefaultRowHeight = 20
    RowCount = 31
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goEditing, goThumbTracking]
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 1
    OnGetEditText = SkillsGridGetEditText
    OnKeyPress = ComponentKeyPress
    OnSetEditText = SkillsGridSetEditText
  end
  object CloseButton: TButton
    Left = 62
    Top = 372
    Width = 107
    Height = 35
    Anchors = [akLeft]
    Caption = '&Close'
    TabOrder = 4
    OnClick = ButtonClick
  end
  object ViewOption: TRadioGroup
    Left = 8
    Top = 326
    Width = 215
    Height = 41
    Anchors = []
    Caption = '&Options'
    Columns = 2
    ItemIndex = 0
    Items.Strings = (
      'View all'
      'View by type')
    TabOrder = 3
    OnClick = ViewOptionClick
  end
  object TabPageCtrl: TPageControl
    Left = 6
    Top = 6
    Width = 219
    Height = 263
    ActivePage = SkillsTab1
    Anchors = [akTop]
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    RaggedRight = True
    TabIndex = 0
    TabOrder = 0
    Visible = False
    object SkillsTab1: TTabSheet
      Caption = 'Tab 1'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      object TabSkillsGrid1: TStringGrid
        Left = 0
        Top = 0
        Width = 211
        Height = 235
        Align = alClient
        ColCount = 2
        DefaultColWidth = 138
        DefaultRowHeight = 20
        RowCount = 11
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goEditing]
        ParentFont = False
        ScrollBars = ssNone
        TabOrder = 0
        OnKeyPress = ComponentKeyPress
        OnSetEditText = TabSkillsGridSetEditText
      end
    end
    object SkillsTab2: TTabSheet
      Caption = 'Tab 2'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ImageIndex = 1
      ParentFont = False
      object TabSkillsGrid2: TStringGrid
        Left = 0
        Top = 0
        Width = 211
        Height = 235
        Align = alClient
        ColCount = 2
        DefaultColWidth = 138
        DefaultRowHeight = 20
        RowCount = 11
        Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goEditing]
        ScrollBars = ssNone
        TabOrder = 0
        OnKeyPress = ComponentKeyPress
        OnSetEditText = TabSkillsGridSetEditText
      end
    end
    object SkillsTab3: TTabSheet
      Caption = 'Tab 3'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ImageIndex = 2
      ParentFont = False
      object TabSkillsGrid3: TStringGrid
        Left = 0
        Top = 0
        Width = 211
        Height = 235
        Align = alClient
        ColCount = 2
        DefaultColWidth = 138
        DefaultRowHeight = 20
        RowCount = 11
        Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goEditing]
        ScrollBars = ssNone
        TabOrder = 0
        OnKeyPress = ComponentKeyPress
        OnSetEditText = TabSkillsGridSetEditText
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
          20)
      end
    end
  end
  object SetSkillsBox: TGroupBox
    Left = 8
    Top = 273
    Width = 215
    Height = 50
    Caption = '&Set all skills to...'
    TabOrder = 2
    object SkillValue: TEdit
      Left = 46
      Top = 18
      Width = 30
      Height = 24
      MaxLength = 2
      TabOrder = 0
      Text = '0'
      OnExit = SkillValueExit
      OnKeyPress = ComponentKeyPress
    end
    object GoButton: TButton
      Left = 111
      Top = 18
      Width = 60
      Height = 25
      Caption = '&Go'
      TabOrder = 1
      OnClick = ButtonClick
    end
    object UpDownControl: TUpDown
      Left = 76
      Top = 18
      Width = 15
      Height = 24
      Associate = SkillValue
      Min = 0
      Max = 20
      Position = 0
      TabOrder = 2
      Thousands = False
      Wrap = False
    end
  end
end
