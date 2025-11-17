#include "Forms/FormLogin.h"
#include "Forms/FormMain.h"

using namespace Client::Forms;
using namespace System::Windows::Forms;
using namespace System::Drawing;

FormLogin::FormLogin()
{
    InitializeComponent();

    // ==== FORM STYLE ====
    this->Text = "Đăng nhập trước khi chơi";
    this->StartPosition = FormStartPosition::CenterScreen;
    this->ClientSize = Size(420, 220);
    this->BackColor = Color::WhiteSmoke;
    this->FormBorderStyle = FormBorderStyle::FixedSingle;
    this->MaximizeBox = false;

    // ==== TITLE ====
    Label^ lblTitle = gcnew Label();
    lblTitle->Text = "Chào mừng bạn đến với Caro Game";
    lblTitle->Font = gcnew Font("Segoe UI", 14, FontStyle::Bold);
    lblTitle->ForeColor = Color::DarkSlateBlue;
    lblTitle->AutoSize = false;
    lblTitle->TextAlign = ContentAlignment::MiddleCenter;
    lblTitle->Dock = DockStyle::Top;
    lblTitle->Height = 50;
    this->Controls->Add(lblTitle);

    // ==== LABEL NHẬP TÊN ====
    Label^ lbl = gcnew Label();
    lbl->Text = "Nhập tên của bạn:";
    lbl->Left = 30;
    lbl->Top = 70;
    lbl->AutoSize = true;
    lbl->Font = gcnew Font("Segoe UI", 11);
    this->Controls->Add(lbl);

    // ==== TEXTBOX ====
    txtName = gcnew TextBox();
    txtName->Left = 30;
    txtName->Top = 100;
    txtName->Width = 350;
    txtName->Font = gcnew Font("Segoe UI", 11);
    this->Controls->Add(txtName);

    // ==== BUTTON ====
    Button^ btn = gcnew Button();
    btn->Text = "Play";
    btn->Left = 30;
    btn->Top = 140;
    btn->Width = 350;
    btn->Height = 40;
    btn->BackColor = Color::MediumSlateBlue;
    btn->ForeColor = Color::White;
    btn->Font = gcnew Font("Segoe UI", 11, FontStyle::Bold);
    btn->FlatStyle = FlatStyle::Flat;
    btn->FlatAppearance->BorderSize = 0;
    this->Controls->Add(btn);

    btn->Click += gcnew EventHandler(this, &FormLogin::BtnPlay_Click);
}

void FormLogin::BtnPlay_Click(Object^ sender, EventArgs^ e)
{
    String^ name = txtName->Text->Trim();
    if (String::IsNullOrWhiteSpace(name))
    {
        MessageBox::Show("Tên không được để trống!");
        return;
    }

    FormMain^ fm = gcnew FormMain(name);
    fm->Show();
    this->Hide();
}
