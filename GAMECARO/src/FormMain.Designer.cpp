#include "Forms/FormMain.h"

using namespace Client::Forms;

void FormMain::InitializeComponent()
{
    this->txtPlayerName = gcnew TextBox();
    this->btnConnect = gcnew Button();
    this->lblYou = gcnew Label();
    this->lblEnemy = gcnew Label();
    this->txtChat = gcnew TextBox();
    this->txtMessage = gcnew TextBox();
    this->btnSend = gcnew Button();
    this->btnSurrender = gcnew Button();

    // txtPlayerName
    this->txtPlayerName->Location = System::Drawing::Point(400, 50);
    this->txtPlayerName->Name = "txtPlayerName";
    this->txtPlayerName->Size = System::Drawing::Size(150, 23);
    this->txtPlayerName->Text = "Nam";

    // btnConnect
    this->btnConnect->Location = System::Drawing::Point(560, 49);
    this->btnConnect->Name = "btnConnect";
    this->btnConnect->Size = System::Drawing::Size(75, 23);
    this->btnConnect->Text = "Connect";
    this->btnConnect->Click += gcnew System::EventHandler(this, &FormMain::BtnConnect_Click);

    // lblYou
    this->lblYou->AutoSize = true;
    this->lblYou->Location = System::Drawing::Point(400, 80);
    this->lblYou->Name = "lblYou";
    this->lblYou->Size = System::Drawing::Size(60, 15);
    this->lblYou->Text = "You: N/A";

    // lblEnemy
    this->lblEnemy->AutoSize = true;
    this->lblEnemy->Location = System::Drawing::Point(400, 100);
    this->lblEnemy->Name = "lblEnemy";
    this->lblEnemy->Size = System::Drawing::Size(76, 15);
    this->lblEnemy->Text = "Enemy: N/A";

    // txtChat
    this->txtChat->Location = System::Drawing::Point(400, 130);
    this->txtChat->Multiline = true;
    this->txtChat->ReadOnly = true;
    this->txtChat->ScrollBars = ScrollBars::Vertical;
    this->txtChat->Size = System::Drawing::Size(370, 380);

    // txtMessage
    this->txtMessage->Location = System::Drawing::Point(400, 520);
    this->txtMessage->Size = System::Drawing::Size(260, 23);

    // btnSend
    this->btnSend->Location = System::Drawing::Point(670, 519);
    this->btnSend->Size = System::Drawing::Size(100, 23);
    this->btnSend->Text = "Send";
    this->btnSend->Click += gcnew System::EventHandler(this, &FormMain::BtnSend_Click);

    // btnSurrender
    this->btnSurrender->Location = System::Drawing::Point(670, 550);
    this->btnSurrender->Size = System::Drawing::Size(100, 23);
    this->btnSurrender->Text = "Đầu hàng";
    this->btnSurrender->Click += gcnew System::EventHandler(this, &FormMain::BtnSurrender_Click);

    // FormMain
    this->AutoScaleDimensions = System::Drawing::SizeF(7, 15);
    this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
    this->ClientSize = System::Drawing::Size(800, 600);
    this->Name = "FormMain";
    this->Text = "Caro Online 15x15";

    // Thêm controls
    this->Controls->Add(this->txtPlayerName);
    this->Controls->Add(this->btnConnect);
    this->Controls->Add(this->lblYou);
    this->Controls->Add(this->lblEnemy);
    this->Controls->Add(this->txtChat);
    this->Controls->Add(this->txtMessage);
    this->Controls->Add(this->btnSend);
    this->Controls->Add(this->btnSurrender);
}
