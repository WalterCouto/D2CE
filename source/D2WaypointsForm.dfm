object WaypointsForm: TWaypointsForm
  Left = 346
  Top = 103
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Waypoints'
  ClientHeight = 327
  ClientWidth = 281
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  DesignSize = (
    281
    327)
  PixelsPerInch = 96
  TextHeight = 16
  object CloseButton: TButton
    Left = 90
    Top = 287
    Width = 100
    Height = 35
    Anchors = [akBottom]
    Caption = '&Close'
    TabOrder = 2
    OnClick = ButtonClick
  end
  object DifficultyPanel: TPanel
    Left = 2
    Top = 192
    Width = 277
    Height = 90
    BevelInner = bvLowered
    BevelOuter = bvSpace
    TabOrder = 1
    object ViewOptions: TRadioGroup
      Left = 4
      Top = 4
      Width = 270
      Height = 41
      Caption = '&Difficulty'
      Columns = 3
      Items.Strings = (
        'Normal'
        'Nightmare'
        'Hell')
      TabOrder = 0
      OnClick = ViewOptionsClick
    end
    object ActivateAllButton: TButton
      Left = 63
      Top = 49
      Width = 150
      Height = 35
      Caption = '&Activate All Waypoints'
      TabOrder = 1
      OnClick = ButtonClick
    end
  end
  object WPPageCtrl: TPageControl
    Left = 1
    Top = 5
    Width = 280
    Height = 180
    ActivePage = Act1Tab
    Anchors = [akLeft, akTop, akRight]
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    RaggedRight = True
    TabIndex = 0
    TabOrder = 0
    object Act1Tab: TTabSheet
      Caption = 'Act I'
      object Act1WP: TCheckListBox
        Left = 0
        Top = 0
        Width = 272
        Height = 149
        OnClickCheck = WPClickCheck
        Align = alClient
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ItemHeight = 16
        Items.Strings = (
          'Rogue Encampment'
          'Cold Plains'
          'Stony Field'
          'Dark Wood'
          'Black Marsh'
          'Outer Cloister'
          'Jail Level 1'
          'Inner Cloister'
          'Catacombs Level 2')
        ParentFont = False
        TabOrder = 0
      end
    end
    object Act2Tab: TTabSheet
      Caption = 'Act II'
      ImageIndex = 1
      object Act2WP: TCheckListBox
        Left = 0
        Top = 0
        Width = 272
        Height = 149
        OnClickCheck = WPClickCheck
        Align = alClient
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ItemHeight = 16
        Items.Strings = (
          'Lut Gholein'
          'Sewers Level 2'
          'Dry Hills'
          'Halls of the Dead Level 2'
          'Far Oasis'
          'Lost City'
          'Palace Cellar Level 1'
          'Arcane Sanctuary'
          'Canyon of the Magi')
        ParentFont = False
        TabOrder = 0
      end
    end
    object Act3Tab: TTabSheet
      Caption = 'Act III'
      ImageIndex = 2
      object Act3WP: TCheckListBox
        Left = 0
        Top = 0
        Width = 272
        Height = 149
        OnClickCheck = WPClickCheck
        Align = alClient
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ItemHeight = 16
        Items.Strings = (
          'Kurast Docks'
          'Spider Forest'
          'Great Marsh'
          'Flayer Jungle'
          'Lower Kurast'
          'Kurast Bazaar'
          'Upper Kurast'
          'Travincal'
          'Durance of Hate Level 2')
        ParentFont = False
        TabOrder = 0
      end
    end
    object Act4Tab: TTabSheet
      Caption = 'Act IV'
      ImageIndex = 3
      object Act4WP: TCheckListBox
        Left = 0
        Top = 0
        Width = 272
        Height = 149
        OnClickCheck = WPClickCheck
        Align = alClient
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ItemHeight = 16
        Items.Strings = (
          'The Pandemonium Fortress'
          'City of the Damned'
          'River of Flames')
        ParentFont = False
        TabOrder = 0
      end
    end
    object Act5Tab: TTabSheet
      Caption = 'Act V'
      ImageIndex = 4
      object Act5WP: TCheckListBox
        Left = 0
        Top = 0
        Width = 272
        Height = 149
        OnClickCheck = WPClickCheck
        Align = alClient
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ItemHeight = 16
        Items.Strings = (
          'Harrogath'
          'Frigid Highlands'
          'Arreat Plateau'
          'Crystalline Passage'
          'Glacial Trail'
          'Halls of Pain'
          'Frozen Tundra'
          'The Ancients'#39' Way'
          'Worldstone Keep Level 2')
        ParentFont = False
        TabOrder = 0
      end
    end
  end
end
