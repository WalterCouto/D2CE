/*
    Diablo II Character Editor
    Copyright (C) 2022 Walter Couto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#include "pch.h"
#include "D2Editor.h"
#include "D2EarAttributesForm.h"
#include "afxdialogex.h"
#include "d2ce/helpers/ItemHelpers.h"
#include <utf8/utf8.h>

//---------------------------------------------------------------------------
// CD2EarAttributesForm dialog

IMPLEMENT_DYNAMIC(CD2EarAttributesForm, CDialogEx)

//---------------------------------------------------------------------------
CD2EarAttributesForm::CD2EarAttributesForm(CD2NewItemForm& form)
    : CDialogEx(CD2EarAttributesForm::IDD, (CWnd*)&form), MainForm(form.MainForm), NewItemFormPtr(&form), ItemPtr(const_cast<d2ce::Item*>(form.CreatedItem))
{
    if (ItemPtr != nullptr)
    {
        ItemPtr->getEarAttributes(EarAttrib);
    }
}
//---------------------------------------------------------------------------
CD2EarAttributesForm::~CD2EarAttributesForm()
{
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHAR_NAME, CharName);
    DDX_Control(pDX, IDC_CHAR_CLASS_CMB, CharClass);
    DDX_Control(pDX, IDC_CHAR_LEVEL, CharLevel);
}
//---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CD2EarAttributesForm, CDialogEx)
    ON_BN_CLICKED(IDOK, &CD2EarAttributesForm::OnBnClickedOk)
END_MESSAGE_MAP()

// CD2EarAttributesForm message handlers

//---------------------------------------------------------------------------
BOOL CD2EarAttributesForm::OnInitDialog()
{
    __super::OnInitDialog();

    {
        std::string strValue;
        std::u16string uText;
        CString text;
        CStringA textA;
        CWnd* pWnd = nullptr;
        if (d2ce::LocalizationHelpers::GetStringTxtValue("ok", strValue))
        {
            pWnd = GetDlgItem(IDOK);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("cancel", strValue))
        {
            pWnd = GetDlgItem(IDCANCEL);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strName", strValue))
        {
            pWnd = GetDlgItem(IDC_STATIC_CHAR_NAME);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strClass", strValue))
        {
            pWnd = GetDlgItem(IDC_STATIC_CHAR_CLASS);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }

        if (d2ce::LocalizationHelpers::GetStringTxtValue("strchrlvl", strValue))
        {
            pWnd = GetDlgItem(IDC_STATIC_CHAR_LEVEL);
            if (pWnd != nullptr)
            {
                pWnd->GetWindowText(text);
                textA = text;
                if (textA.CompareNoCase(strValue.c_str()) != 0)
                {
                    uText = utf8::utf8to16(strValue);
                    pWnd->SetWindowText(reinterpret_cast<LPCWSTR>(uText.c_str()));
                }
            }
        }
    } 
    
    // Set text limite on edit boxes
    CharName.SetLimitText(15);
    CharLevel.SetLimitText(2);

    SetUTF8Text(&CharName, EarAttrib.getName().data());
    UpdateClassDisplay();
    SetInt(&CharLevel, EarAttrib.getLevel());

    return TRUE;  // return TRUE unless you set the focus to a control
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::OnBnClickedOk()
{
    UpdateData(TRUE);
    if (ItemPtr != nullptr)
    {
        auto newName = ToStdString(&CharName);
        strcpy_s(EarAttrib.Name.data(), sizeof(EarAttrib.Name), newName.c_str());
        EarAttrib.Name[EarAttrib.Name.size() - 1] = 0;

        EarAttrib.Class = static_cast<d2ce::EnumCharClass>(CharClass.GetCurSel());

        EarAttrib.Level = std::min(std::max(ToInt(&CharLevel), std::uint32_t(1)), d2ce::MAX_NUM_LEVELS);

        ItemPtr->setEarAttributes(EarAttrib);
    }
    __super::OnOK();
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::UpdateClassDisplay()
{
    auto curClass = EarAttrib.getClass();
    CharClass.ResetContent();
    std::u16string uText;
    for (const auto& type : d2ce::LocalizationHelpers::GetCharacterTypes(MainForm.isExpansionCharacter()))
    {
        uText = utf8::utf8to16(type);
        CharClass.AddString(CString(reinterpret_cast<LPCWSTR>(uText.c_str())));
    }

    CharClass.SetCurSel(static_cast<std::underlying_type_t<d2ce::EnumCharClass>>(curClass));
}
//---------------------------------------------------------------------------
std::string CD2EarAttributesForm::ToStdString(const CWnd* Sender) const
{
    CStringW wValue(ToText(Sender));
    return utf8::utf16to8(reinterpret_cast<const char16_t*>(wValue.GetString()));
}
//---------------------------------------------------------------------------
CString CD2EarAttributesForm::ToText(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText;
        Sender->GetWindowText(sWindowText);
        return sWindowText;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        CString sWindowText;
        ((CComboBox*)Sender)->GetLBText(((CComboBox*)Sender)->GetCurSel(), sWindowText);
        return sWindowText;
    }

    return CString();
}
//---------------------------------------------------------------------------
CStringA CD2EarAttributesForm::ToTextA(const CWnd* Sender) const
{
    return CStringA(ToText(Sender));
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::SetText(CWnd* Sender, const std::string& newValue)
{
    SetUTF8Text(Sender, newValue.c_str());
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::SetUTF8Text(CWnd* Sender, const char* newValue)
{
    auto uText = utf8::utf8to16(newValue);
    SetText(Sender, reinterpret_cast<LPCWSTR>(uText.c_str()));
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::SetText(CWnd* Sender, const char* newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText(newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
    }
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::SetText(CWnd* Sender, const wchar_t* newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText(newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
    }
}
//---------------------------------------------------------------------------
std::uint32_t CD2EarAttributesForm::ToInt(const CWnd* Sender) const
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        std::string sWindowText(ToStdString(Sender));
        char* ptr = nullptr;
        std::uint32_t value = std::strtoul(sWindowText.c_str(), &ptr, 10);
        return value;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        return ((CComboBox*)Sender)->GetCurSel();
    }

    return 0;
}
//---------------------------------------------------------------------------
void CD2EarAttributesForm::SetInt(CWnd* Sender, std::uint32_t newValue)
{
    if (Sender->IsKindOf(RUNTIME_CLASS(CEdit)) || Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
    {
        CString sWindowText;
        sWindowText.Format(_T("%lu"), newValue);
        Sender->SetWindowText(sWindowText);
        if (Sender->IsKindOf(RUNTIME_CLASS(CStatic)))
        {
            CRect rect;
            Sender->GetWindowRect(&rect);
            ScreenToClient(&rect);
            InvalidateRect(&rect);
            UpdateWindow();
        }
        return;
    }

    if (Sender->IsKindOf(RUNTIME_CLASS(CComboBox)))
    {
        ((CComboBox*)Sender)->SetCurSel(newValue);
        return;
    }
}
//---------------------------------------------------------------------------