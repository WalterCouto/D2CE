object MainForm: TMainForm
  Left = 521
  Top = 133
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  ClientHeight = 340
  ClientWidth = 495
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu
  OldCreateOrder = False
  Position = poScreenCenter
  Visible = True
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object CharInfoBox: TGroupBox
    Left = 0
    Top = 0
    Width = 495
    Height = 322
    Align = alClient
    Caption = 'Character Info'
    Enabled = False
    TabOrder = 0
    Visible = False
    DesignSize = (
      495
      322)
    object lblName: TLabel
      Left = 165
      Top = 18
      Width = 28
      Height = 13
      Caption = 'Name'
    end
    object lblClass: TLabel
      Left = 293
      Top = 18
      Width = 25
      Height = 13
      Caption = 'Class'
    end
    object lblStatus: TLabel
      Left = 405
      Top = 18
      Width = 30
      Height = 13
      Caption = 'Status'
    end
    object lblTitle: TLabel
      Left = 48
      Top = 18
      Width = 20
      Height = 13
      Caption = 'Title'
    end
    object lblStartingAct: TLabel
      Left = 213
      Top = 67
      Width = 55
      Height = 13
      Caption = 'Starting Act'
    end
    object lblDifficulty: TLabel
      Left = 26
      Top = 67
      Width = 102
      Height = 13
      AutoSize = False
      Caption = 'Difficulty Last Played:'
      WordWrap = True
    end
    object Difficulty: TLabel
      Left = 130
      Top = 67
      Width = 54
      Height = 13
      AutoSize = False
      Caption = 'n/a'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsItalic]
      ParentFont = False
    end
    object CharName: TEdit
      Left = 126
      Top = 32
      Width = 115
      Height = 21
      AutoSelect = False
      MaxLength = 15
      TabOrder = 1
      OnChange = CharStatsChange
      OnEnter = CharNameEnter
      OnExit = CharNameExit
      OnKeyPress = CharNameKeyPress
    end
    object StatsBox: TGroupBox
      Left = 10
      Top = 92
      Width = 475
      Height = 178
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Character stats'
      TabOrder = 5
      object lblLevel: TLabel
        Left = 21
        Top = 19
        Width = 26
        Height = 13
        Caption = 'Level'
      end
      object lblStrength: TLabel
        Left = 7
        Top = 56
        Width = 40
        Height = 13
        Caption = 'Strength'
      end
      object lblEnergy: TLabel
        Left = 14
        Top = 134
        Width = 33
        Height = 13
        Caption = 'Energy'
      end
      object lblDexterity: TLabel
        Left = 7
        Top = 82
        Width = 41
        Height = 13
        Caption = 'Dexterity'
      end
      object lblVitality: TLabel
        Left = 17
        Top = 109
        Width = 30
        Height = 13
        Caption = 'Vitality'
      end
      object lblExperience: TLabel
        Left = 111
        Top = 18
        Width = 53
        Height = 13
        Caption = 'Experience'
      end
      object CharLevel: TEdit
        Left = 55
        Top = 15
        Width = 19
        Height = 21
        AutoSelect = False
        MaxLength = 2
        TabOrder = 0
        OnChange = CharStatsChange
        OnEnter = StatsEnter
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object CharStrength: TEdit
        Left = 55
        Top = 52
        Width = 43
        Height = 21
        AutoSelect = False
        MaxLength = 5
        ParentShowHint = False
        ShowHint = False
        TabOrder = 2
        OnChange = CharStatsChange
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object CharEnergy: TEdit
        Left = 55
        Top = 131
        Width = 43
        Height = 21
        AutoSelect = False
        MaxLength = 5
        ParentShowHint = False
        ShowHint = False
        TabOrder = 5
        OnChange = CharStatsChange
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object CharDexterity: TEdit
        Left = 55
        Top = 78
        Width = 43
        Height = 21
        AutoSelect = False
        MaxLength = 5
        ParentShowHint = False
        ShowHint = False
        TabOrder = 3
        OnChange = CharStatsChange
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object CharVitality: TEdit
        Left = 55
        Top = 104
        Width = 43
        Height = 21
        AutoSelect = False
        MaxLength = 5
        ParentShowHint = False
        ShowHint = False
        TabOrder = 4
        OnChange = CharStatsChange
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object LifeBox: TGroupBox
        Left = 110
        Top = 40
        Width = 180
        Height = 40
        Caption = 'Life'
        TabOrder = 6
        object lblCurLife: TLabel
          Left = 10
          Top = 16
          Width = 34
          Height = 13
          Caption = 'Current'
        end
        object lblMaximum: TLabel
          Left = 90
          Top = 16
          Width = 44
          Height = 13
          Caption = 'Maximum'
        end
        object CurLife: TEdit
          Left = 51
          Top = 12
          Width = 31
          Height = 21
          AutoSelect = False
          MaxLength = 4
          TabOrder = 0
          OnChange = CharStatsChange
          OnExit = CharStatsExitCheck
          OnKeyPress = CharStatsKeyPress
        end
        object MaxLife: TEdit
          Left = 140
          Top = 12
          Width = 31
          Height = 21
          AutoSelect = False
          MaxLength = 4
          TabOrder = 1
          OnChange = CharStatsChange
          OnExit = CharStatsExitCheck
          OnKeyPress = CharStatsKeyPress
        end
      end
      object ManaBox: TGroupBox
        Left = 110
        Top = 81
        Width = 180
        Height = 40
        Caption = 'Mana'
        TabOrder = 7
        object lblCurMana: TLabel
          Left = 10
          Top = 16
          Width = 34
          Height = 13
          Caption = 'Current'
        end
        object lblMaxMana: TLabel
          Left = 90
          Top = 16
          Width = 44
          Height = 13
          Caption = 'Maximum'
        end
        object CurMana: TEdit
          Left = 50
          Top = 12
          Width = 31
          Height = 21
          AutoSelect = False
          MaxLength = 4
          TabOrder = 0
          OnChange = CharStatsChange
          OnExit = CharStatsExitCheck
          OnKeyPress = CharStatsKeyPress
        end
        object MaxMana: TEdit
          Left = 140
          Top = 12
          Width = 31
          Height = 21
          AutoSelect = False
          MaxLength = 4
          TabOrder = 1
          OnChange = CharStatsChange
          OnExit = CharStatsExitCheck
          OnKeyPress = CharStatsKeyPress
        end
      end
      object StaminaBox: TGroupBox
        Left = 110
        Top = 122
        Width = 180
        Height = 40
        Caption = 'Stamina'
        TabOrder = 8
        object lblCurStamina: TLabel
          Left = 10
          Top = 16
          Width = 34
          Height = 13
          Caption = 'Current'
        end
        object lblMaxStamina: TLabel
          Left = 90
          Top = 16
          Width = 44
          Height = 13
          Caption = 'Maximum'
        end
        object CurStamina: TEdit
          Left = 50
          Top = 12
          Width = 31
          Height = 21
          AutoSelect = False
          MaxLength = 4
          TabOrder = 0
          OnChange = CharStatsChange
          OnExit = CharStatsExitCheck
          OnKeyPress = CharStatsKeyPress
        end
        object MaxStamina: TEdit
          Left = 140
          Top = 12
          Width = 31
          Height = 21
          AutoSelect = False
          MaxLength = 4
          TabOrder = 1
          OnChange = CharStatsChange
          OnExit = CharStatsExitCheck
          OnKeyPress = CharStatsKeyPress
        end
      end
      object Experience: TEdit
        Left = 172
        Top = 15
        Width = 68
        Height = 21
        AutoSelect = False
        MaxLength = 10
        TabOrder = 1
        OnChange = CharStatsChange
        OnEnter = ExperienceEnter
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object FixSkillButton: TButton
        Left = 307
        Top = 13
        Width = 150
        Height = 35
        Hint = 'Resets skill choices remaining to zero'
        Caption = '&Remove "New Skills" Button'
        TabOrder = 9
        OnClick = ButtonClick
      end
      object WaypointsButton: TButton
        Left = 336
        Top = 94
        Width = 90
        Height = 35
        Caption = '&Waypoints'
        TabOrder = 11
        OnClick = ButtonClick
      end
      object EditSkillsButton: TButton
        Left = 336
        Top = 134
        Width = 90
        Height = 35
        Caption = '&Skill Trees'
        TabOrder = 12
        OnClick = ButtonClick
      end
      object QuestsButton: TButton
        Left = 336
        Top = 54
        Width = 90
        Height = 34
        Caption = '&Quests'
        TabOrder = 10
        OnClick = ButtonClick
      end
    end
    object CharClass: TComboBox
      Left = 266
      Top = 32
      Width = 90
      Height = 21
      Style = csDropDownList
      DropDownCount = 5
      ItemHeight = 13
      MaxLength = 11
      TabOrder = 2
      OnChange = ComboBoxChange
      Items.Strings = (
        'Amazon'
        'Sorceress'
        'Necromancer'
        'Paladin'
        'Barbarian'
        'Druid'
        'Assassin')
    end
    object CharStatus: TComboBox
      Left = 382
      Top = 32
      Width = 85
      Height = 21
      Style = csDropDownList
      DropDownCount = 4
      ItemHeight = 13
      TabOrder = 3
      OnChange = ComboBoxChange
      Items.Strings = (
        'No Deaths'
        'Hardcore'
        'Resurrected'
        'Dead')
    end
    object CharTitle: TComboBox
      Left = 28
      Top = 32
      Width = 75
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 0
      OnChange = ComboBoxChange
    end
    object StartingAct: TComboBox
      Left = 273
      Top = 63
      Width = 195
      Height = 21
      Style = csDropDownList
      ItemHeight = 13
      TabOrder = 4
      OnChange = ComboBoxChange
      Items.Strings = (
        'Act I -- Rogue Encampment'
        'Act II -- Lut Gholein'
        'Act III -- Kurast Docks'
        'Act IV -- The Pandemonium Fortress'
        'Act V -- Harrogath')
    end
    object MaxAllButton: TButton
      Left = 371
      Top = 278
      Width = 100
      Height = 35
      Hint = 'Max all stats including all skills'
      Caption = '&Max Everything'
      TabOrder = 8
      OnClick = ButtonClick
    end
    object GoldBox: TGroupBox
      Left = 10
      Top = 272
      Width = 230
      Height = 43
      Caption = 'Gold'
      TabOrder = 6
      object lblBelt: TLabel
        Left = 10
        Top = 18
        Width = 30
        Height = 13
        Caption = 'In Belt'
      end
      object lblStash: TLabel
        Left = 125
        Top = 18
        Width = 39
        Height = 13
        Caption = 'In Stash'
      end
      object GoldInBelt: TEdit
        Left = 46
        Top = 14
        Width = 50
        Height = 21
        AutoSelect = False
        MaxLength = 6
        TabOrder = 0
        OnChange = CharStatsChange
        OnEnter = StatsEnter
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
      object GoldInStash: TEdit
        Left = 169
        Top = 14
        Width = 50
        Height = 21
        AutoSelect = False
        MaxLength = 7
        TabOrder = 1
        OnChange = CharStatsChange
        OnEnter = StatsEnter
        OnExit = CharStatsExitCheck
        OnKeyPress = CharStatsKeyPress
      end
    end
    object UpgradeGemsButton: TButton
      Left = 261
      Top = 278
      Width = 90
      Height = 35
      Hint = 'Upgrade all gems to their perfect state'
      Caption = '&Upgrade Gems'
      TabOrder = 7
      OnClick = ButtonClick
    end
  end
  object StatusBar: TStatusBar
    Left = 0
    Top = 322
    Width = 495
    Height = 18
    Panels = <
      item
        Width = 480
      end
      item
        Alignment = taCenter
        Width = 50
      end>
    SimplePanel = False
    SizeGrip = False
  end
  object OpenCharDlg: TOpenDialog
    DefaultExt = 'd2s'
    FileName = '*.d2s'
    Filter = 'Diablo 2 Character Files (*.d2s)|*.d2s'
    Left = 444
    Top = 237
  end
  object MainMenu: TMainMenu
    Left = 444
    Top = 174
    object FileMenuItem: TMenuItem
      Caption = '&File'
      object OpenItem: TMenuItem
        Action = FileOpenAction
      end
      object SaveItem: TMenuItem
        Action = FileSaveAction
      end
      object CloseItem: TMenuItem
        Action = FileCloseAction
      end
      object N1: TMenuItem
        Action = FileSeparatorAction
      end
      object ExitItem: TMenuItem
        Action = FileExitAction
      end
    end
    object EditMenuItem: TMenuItem
      Caption = '&Edit'
      object Undo: TMenuItem
        Action = EditUndoAction
      end
    end
    object ViewMenuItem: TMenuItem
      Caption = '&View'
      object LevelReqItem: TMenuItem
        Action = LevelInfoAction
      end
    end
    object OptionsMenuItem: TMenuItem
      Caption = '&Options'
      object BackupCharItem: TMenuItem
        Action = BackupAction
      end
      object RestoreCharacterItem: TMenuItem
        Action = RestoreAction
      end
      object N3: TMenuItem
        Caption = '-'
      end
      object CheckCharItem: TMenuItem
        Action = CheckCharAction
      end
      object GemConverterItem: TMenuItem
        Caption = '&GPS Converter'
        Enabled = False
        OnClick = GemConverterItemClick
      end
    end
    object HelpMenuItem: TMenuItem
      Caption = '&Help'
      object AboutItem: TMenuItem
        Action = HelpAboutAction
      end
    end
  end
  object D2ActionList: TActionList
    Left = 443
    Top = 206
    object FileOpenAction: TAction
      Category = 'File'
      Caption = '&Open'
      Hint = 'Open character'
      ShortCut = 16463
      OnExecute = FileOpenActionExecute
    end
    object FileSaveAction: TAction
      Category = 'File'
      Caption = '&Save'
      Enabled = False
      Hint = 'Save edited character'
      ShortCut = 16467
      OnExecute = FileSaveActionExecute
    end
    object FileCloseAction: TAction
      Category = 'File'
      Caption = '&Close'
      Enabled = False
      Hint = 'Close current file'
      ShortCut = 16451
      OnExecute = FileCloseActionExecute
    end
    object FileSeparatorAction: TAction
      Category = 'File'
      Caption = '-'
    end
    object FileExitAction: TAction
      Category = 'File'
      Caption = 'E&xit'
      Hint = 'Exit editor'
      OnExecute = FileExitActionExecute
    end
    object HelpAboutAction: TAction
      Category = 'Help'
      Caption = '&About'
      OnExecute = HelpAboutActionExecute
    end
    object EditUndoAction: TAction
      Category = 'Edit'
      Caption = '&Undo'
      Enabled = False
      Hint = 'Revert all stats back to previous values'
      ShortCut = 16474
      OnExecute = EditUndoActionExecute
    end
    object LevelInfoAction: TAction
      Category = 'View'
      Caption = '&Level Requirements'
      Hint = 'Displays level requirements and gold limits'
      OnExecute = LevelInfoActionExecute
    end
    object BackupAction: TAction
      Category = 'Options'
      Caption = '&Backup Character'
      Checked = True
      Hint = 'Creates a backup file of the opened character file before saving'
      OnExecute = BackupActionExecute
    end
    object CheckCharAction: TAction
      Category = 'Options'
      Caption = '&Check Character'
      Enabled = False
      Hint = 'Checks your character'#39's stats for any discrepancies'
      OnExecute = CheckCharActionExecute
    end
    object RestoreAction: TAction
      Category = 'Options'
      Caption = '&Restore Character'
      Enabled = False
      Hint = 'Restores a character from a previous backup'
      OnExecute = RestoreActionExecute
    end
  end
end
