#include "Forms/FormLogin.h"

using namespace Client::Forms;

void FormLogin::InitializeComponent()
{
    this->components = gcnew System::ComponentModel::Container();
    this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
    this->ClientSize = System::Drawing::Size(800, 450);
    this->Text = "FormLogin";
}
