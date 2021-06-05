object QuestsForm: TQuestsForm
  Left = 338
  Top = 103
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Quests'
  ClientHeight = 367
  ClientWidth = 376
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poMainFormCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 16
  object CloseButton: TButton
    Left = 99
    Top = 330
    Width = 178
    Height = 33
    Caption = '&Close'
    TabOrder = 2
    OnClick = ButtonClick
  end
  object DifficultyPanel: TPanel
    Left = 38
    Top = 235
    Width = 300
    Height = 90
    BevelInner = bvLowered
    BevelOuter = bvSpace
    TabOrder = 1
    object Difficulty: TRadioGroup
      Left = 8
      Top = 3
      Width = 285
      Height = 40
      Caption = '&Difficulty'
      Columns = 3
      Items.Strings = (
        'Normal'
        'Nightmare'
        'Hell')
      TabOrder = 0
      OnClick = RadioClick
    end
    object CompleteAllButton: TButton
      Left = 61
      Top = 48
      Width = 177
      Height = 35
      Caption = 'Complete All &Quests'
      TabOrder = 1
      OnClick = ButtonClick
    end
  end
  object QuestsPage: TPageControl
    Left = 4
    Top = 3
    Width = 368
    Height = 227
    ActivePage = Act5Tab
    TabIndex = 4
    TabOrder = 0
    OnChange = QuestsPageChange
    object Act1Tab: TTabSheet
      Caption = 'Act I'
      object Act1Panel: TPanel
        Left = 0
        Top = 0
        Width = 360
        Height = 196
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
        object A1Q1Label: TLabel
          Left = 10
          Top = 7
          Width = 77
          Height = 16
          Caption = '1. Den of Evil'
          FocusControl = A1Q1
        end
        object A1Q2Label: TLabel
          Left = 188
          Top = 7
          Width = 148
          Height = 16
          Caption = '2. Sisters'#39' Burial Grounds'
          FocusControl = A1Q2
        end
        object A1Q3Label: TLabel
          Left = 10
          Top = 55
          Width = 131
          Height = 16
          Caption = '3. The Search for Cain'
          FocusControl = A1Q3
        end
        object A1Q4Label: TLabel
          Left = 188
          Top = 55
          Width = 138
          Height = 16
          Caption = '4. The Forgotten Tower'
          FocusControl = A1Q4
        end
        object A1Q5Label: TLabel
          Left = 10
          Top = 104
          Width = 123
          Height = 16
          Caption = '5. Tools of the Trade'
          FocusControl = A1Q5
        end
        object A1Q6Label: TLabel
          Left = 188
          Top = 104
          Width = 149
          Height = 16
          Caption = '6. Sisters to the Slaughter'
          FocusControl = A1Q6
        end
        object A1Q1: TComboBox
          Left = 10
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 0
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A1Q2: TComboBox
          Left = 188
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 1
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A1Q3: TComboBox
          Left = 10
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 2
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A1Q4: TComboBox
          Left = 188
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 3
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A1Q5: TComboBox
          Left = 10
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 4
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Imbue'
            'Completed')
        end
        object A1Q6: TComboBox
          Left = 188
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 5
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
      end
    end
    object Act2Tab: TTabSheet
      Caption = 'Act II'
      ImageIndex = 1
      object Act2Panel: TPanel
        Left = 0
        Top = 0
        Width = 360
        Height = 196
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
        object Label1: TLabel
          Left = 10
          Top = 7
          Width = 111
          Height = 16
          Caption = '1. Radament'#39's Lair'
        end
        object Label2: TLabel
          Left = 188
          Top = 7
          Width = 121
          Height = 16
          Caption = '2. The Horadric Staff'
        end
        object Label3: TLabel
          Left = 10
          Top = 55
          Width = 85
          Height = 16
          Caption = '3. Tainted Sun'
        end
        object Label4: TLabel
          Left = 188
          Top = 55
          Width = 119
          Height = 16
          Caption = '4. Arcane Sanctuary'
        end
        object Label5: TLabel
          Left = 10
          Top = 104
          Width = 105
          Height = 16
          Caption = '5. The Summoner'
        end
        object Label6: TLabel
          Left = 188
          Top = 104
          Width = 125
          Height = 16
          Caption = '6. The Seven Tombs'
        end
        object A2Q1: TComboBox
          Left = 10
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 0
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A2Q2: TComboBox
          Left = 188
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 1
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A2Q3: TComboBox
          Left = 10
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 2
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A2Q4: TComboBox
          Left = 188
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 3
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A2Q5: TComboBox
          Left = 10
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 4
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A2Q6: TComboBox
          Left = 188
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 5
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object Act2Status: TRadioGroup
          Left = 85
          Top = 150
          Width = 190
          Height = 40
          Caption = '&Act Status'
          Columns = 2
          Items.Strings = (
            'Disabled'
            'Enabled')
          TabOrder = 6
          OnClick = RadioClick
          OnEnter = ActStatusEnter
          OnExit = ActStatusExit
        end
      end
    end
    object Act3Tab: TTabSheet
      Caption = 'Act III'
      ImageIndex = 2
      object Act3Panel: TPanel
        Left = 0
        Top = 0
        Width = 360
        Height = 196
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
        object Label7: TLabel
          Left = 10
          Top = 7
          Width = 111
          Height = 16
          Caption = '1. The Golden Bird'
        end
        object Label8: TLabel
          Left = 188
          Top = 7
          Width = 161
          Height = 16
          Caption = '2. Blade of the Old Religion'
        end
        object Label9: TLabel
          Left = 10
          Top = 55
          Width = 88
          Height = 16
          Caption = '3. Khalim'#39's Will'
        end
        object Label10: TLabel
          Left = 188
          Top = 55
          Width = 122
          Height = 16
          Caption = '4. Lam Esen'#39's Tome'
        end
        object Label11: TLabel
          Left = 10
          Top = 104
          Width = 155
          Height = 16
          Caption = '5. The Blackened Temple'
        end
        object Label12: TLabel
          Left = 188
          Top = 104
          Width = 95
          Height = 16
          Caption = '6. The Guardian'
        end
        object A3Q1: TComboBox
          Left = 10
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 0
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A3Q2: TComboBox
          Left = 188
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 1
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A3Q3: TComboBox
          Left = 10
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 2
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A3Q4: TComboBox
          Left = 188
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 3
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A3Q5: TComboBox
          Left = 10
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 4
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A3Q6: TComboBox
          Left = 188
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 5
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object Act3Status: TRadioGroup
          Left = 85
          Top = 150
          Width = 190
          Height = 40
          Caption = '&Act Status'
          Columns = 2
          Items.Strings = (
            'Disabled'
            'Enabled')
          TabOrder = 6
          OnClick = RadioClick
          OnEnter = ActStatusEnter
          OnExit = ActStatusExit
        end
      end
    end
    object Act4Tab: TTabSheet
      Caption = 'Act IV'
      ImageIndex = 3
      object Act4Panel: TPanel
        Left = 0
        Top = 0
        Width = 360
        Height = 196
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
        object Label13: TLabel
          Left = 10
          Top = 7
          Width = 115
          Height = 16
          Caption = '1. The Fallen Angel'
        end
        object Label14: TLabel
          Left = 188
          Top = 7
          Width = 86
          Height = 16
          Caption = '2. Hell'#39's Forge'
        end
        object Label15: TLabel
          Left = 10
          Top = 55
          Width = 87
          Height = 16
          Caption = '3. Terror'#39's End'
        end
        object A4Q1: TComboBox
          Left = 10
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 0
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A4Q2: TComboBox
          Left = 188
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 1
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A4Q3: TComboBox
          Left = 10
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 2
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object Act4Status: TRadioGroup
          Left = 85
          Top = 150
          Width = 190
          Height = 40
          Caption = '&Act Status'
          Columns = 2
          Items.Strings = (
            'Disabled'
            'Enabled')
          TabOrder = 3
          OnClick = RadioClick
          OnEnter = ActStatusEnter
          OnExit = ActStatusExit
        end
      end
    end
    object Act5Tab: TTabSheet
      Caption = 'Act V'
      ImageIndex = 4
      object Act5Panel: TPanel
        Left = 0
        Top = 0
        Width = 360
        Height = 196
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
        object Label19: TLabel
          Left = 10
          Top = 7
          Width = 130
          Height = 16
          Caption = '1. Siege on Harrogath'
        end
        object Label20: TLabel
          Left = 188
          Top = 7
          Width = 156
          Height = 16
          Caption = '2. Rescue on Mount Arreat'
        end
        object Label21: TLabel
          Left = 10
          Top = 55
          Width = 86
          Height = 16
          Caption = '3. Prison of Ice'
        end
        object Label22: TLabel
          Left = 188
          Top = 55
          Width = 140
          Height = 16
          Caption = '4. Betrayal of Harrogath'
        end
        object Label23: TLabel
          Left = 10
          Top = 104
          Width = 109
          Height = 16
          Caption = '5. Rite of Passage'
        end
        object Label24: TLabel
          Left = 188
          Top = 104
          Width = 121
          Height = 16
          Caption = '6. Eve of Destruction'
        end
        object A5Q1: TComboBox
          Left = 10
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 0
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Add Sockets'
            'Completed')
        end
        object A5Q2: TComboBox
          Left = 188
          Top = 25
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 1
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Ort, Ral, Tal Runes'
            'Completed')
        end
        object A5Q3: TComboBox
          Left = 10
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 2
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Scroll of Resistance'
            'Completed')
        end
        object A5Q4: TComboBox
          Left = 188
          Top = 72
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 3
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Personalize'
            'Completed')
        end
        object A5Q5: TComboBox
          Left = 10
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 4
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object A5Q6: TComboBox
          Left = 188
          Top = 123
          Width = 162
          Height = 24
          Style = csDropDownList
          ItemHeight = 16
          TabOrder = 5
          OnChange = QuestsChange
          Items.Strings = (
            'Not Started'
            'Started/In Progress'
            'Completed')
        end
        object Act5Status: TRadioGroup
          Left = 85
          Top = 150
          Width = 190
          Height = 40
          Caption = '&Act Status'
          Columns = 2
          Items.Strings = (
            'Disabled'
            'Enabled')
          TabOrder = 6
          OnClick = RadioClick
          OnEnter = ActStatusEnter
          OnExit = ActStatusExit
        end
      end
    end
  end
end
