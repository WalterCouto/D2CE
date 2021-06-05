object ConvertGemsForm: TConvertGemsForm
  Left = 273
  Top = 104
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'GPS Converter'
  ClientHeight = 140
  ClientWidth = 257
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
  object ButtonPanel: TPanel
    Left = 38
    Top = 82
    Width = 180
    Height = 37
    BevelOuter = bvNone
    TabOrder = 0
    object ConvertButton: TButton
      Left = 4
      Top = 4
      Width = 80
      Height = 30
      Caption = 'C&onvert'
      TabOrder = 0
      OnClick = ConvertButtonClick
    end
    object CloseButton: TButton
      Left = 96
      Top = 4
      Width = 80
      Height = 30
      Cancel = True
      Caption = '&Close'
      TabOrder = 1
      OnClick = CloseButtonClick
    end
  end
  object Panel1: TPanel
    Left = 7
    Top = 4
    Width = 244
    Height = 75
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 1
    object Label1: TLabel
      Left = 10
      Top = 16
      Width = 46
      Height = 16
      Caption = 'Convert'
    end
    object Label2: TLabel
      Left = 45
      Top = 43
      Width = 11
      Height = 16
      Caption = 'to'
    end
    object OldGem: TComboBox
      Left = 60
      Top = 12
      Width = 173
      Height = 24
      Style = csDropDownList
      ItemHeight = 16
      TabOrder = 0
      Items.Strings = (
        'Chipped Amethysts'
        'Chipped Diamonds'
        'Chipped Emeralds'
        'Chipped Rubies'
        'Chipped Sapphires'
        'Chipped Skulls'
        'Chipped Topazes'
        'Flawed Amethysts'
        'Flawed Diamonds'
        'Flawed Emeralds'
        'Flawed Rubies'
        'Flawed Sapphires'
        'Flawed Skulls'
        'Flawed Topazes'
        'Regular Amethysts'
        'Regular Diamonds'
        'Regular Emeralds'
        'Regular Rubies'
        'Regular Sapphires'
        'Regular Skulls'
        'Regular Topazes'
        'Flawless Amethysts'
        'Flawless Diamonds'
        'Flawless Emeralds'
        'Flawless Rubies'
        'Flawless Sapphires'
        'Flawless Skulls'
        'Flawless Topazes'
        'Perfect Amethysts'
        'Perfect Diamonds'
        'Perfect Emeralds'
        'Perfect Rubies'
        'Perfect Sapphires'
        'Perfect Skulls'
        'Perfect Topazes'
        'Minor Healing Potion'
        'Light Healing Potion'
        'Healing Potion'
        'Greater Healing Potion'
        'Super Healing Potion'
        'Minor Mana Potion'
        'Light Mana Potion'
        'Mana Potion'
        'Greater Mana Potion'
        'Super Mana Potion'
        'Rejuvenation Potion'
        'Full Rejuvenation Potion'
        'Antidote Potion'
        'Stamina Potion'
        'Thawing Potion')
    end
    object NewGem: TComboBox
      Left = 60
      Top = 40
      Width = 173
      Height = 24
      Style = csDropDownList
      ItemHeight = 16
      TabOrder = 1
      Items.Strings = (
        'Chipped Amethysts'
        'Chipped Diamonds'
        'Chipped Emeralds'
        'Chipped Rubies'
        'Chipped Sapphires'
        'Chipped Skulls'
        'Chipped Topazes'
        'Flawed Amethysts'
        'Flawed Diamonds'
        'Flawed Emeralds'
        'Flawed Rubies'
        'Flawed Sapphires'
        'Flawed Skulls'
        'Flawed Topazes'
        'Regular Amethysts'
        'Regular Diamonds'
        'Regular Emeralds'
        'Regular Rubies'
        'Regular Sapphires'
        'Regular Skulls'
        'Regular Topazes'
        'Flawless Amethysts'
        'Flawless Diamonds'
        'Flawless Emeralds'
        'Flawless Rubies'
        'Flawless Sapphires'
        'Flawless Skulls'
        'Flawless Topazes'
        'Perfect Amethysts'
        'Perfect Diamonds'
        'Perfect Emeralds'
        'Perfect Rubies'
        'Perfect Sapphires'
        'Perfect Skulls'
        'Perfect Topazes'
        'Minor Healing Potion'
        'Light Healing Potion'
        'Healing Potion'
        'Greater Healing Potion'
        'Super Healing Potion'
        'Minor Mana Potion'
        'Light Mana Potion'
        'Mana Potion'
        'Greater Mana Potion'
        'Super Mana Potion'
        'Rejuvenation Potion'
        'Full Rejuvenation Potion'
        'Antidote Potion'
        'Stamina Potion'
        'Thawing Potion')
    end
  end
  object GemStatusBar: TStatusBar
    Left = 0
    Top = 120
    Width = 257
    Height = 20
    Panels = <>
    SimplePanel = False
  end
end
