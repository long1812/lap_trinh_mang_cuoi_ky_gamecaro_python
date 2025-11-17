#pragma once

using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace Client {
namespace Forms {

    public ref class FormMain : public Form
    {
    public:
        FormMain()
        {
            InitializeComponent();
        }

    private:
        TextBox^ txtPlayerName;
        Button^ btnConnect;
        Label^ lblYou;
        Label^ lblEnemy;
        TextBox^ txtChat;
        TextBox^ txtMessage;
        Button^ btnSend;
        Button^ btnSurrender;

        void InitializeComponent()
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
            this->txtPlayerName->Location = Point(400, 50);
            this->txtPlayerName->Name = "txtPlayerName";
            this->txtPlayerName->Size = Size(150, 23);
            this->txtPlayerName->Text = "Nam";

            // btnConnect
            this->btnConnect->Location = Point(560, 49);
            this->btnConnect->Name = "btnConnect";
            this->btnConnect->Size = Size(75, 23);
            this->btnConnect->Text = "Connect";
            this->btnConnect->UseVisualStyleBackColor = true;
            this->btnConnect->Click += gcnew EventHandler(this, &FormMain::BtnConnect_Click);

            // lblYou
            this->lblYou->AutoSize = true;
            this->lblYou->Location = Point(400, 80);
            this->lblYou->Name = "lblYou";
            this->lblYou->Size = Size(60, 15);
            this->lblYou->Text = "You: N/A";

            // lblEnemy
            this->lblEnemy->AutoSize = true;
            this->lblEnemy->Location = Point(400, 100);
            this->lblEnemy->Name = "lblEnemy";
            this->lblEnemy->Size = Size(76, 15);
            this->lblEnemy->Text = "Enemy: N/A";

            // txtChat
            this->txtChat->Location = Point(400, 130);
            this->txtChat->Multiline = true;
            this->txtChat->ReadOnly = true;
            this->txtChat->ScrollBars = ScrollBars::Vertical;
            this->txtChat->Size = Size(370, 380);

            // txtMessage
            this->txtMessage->Location = Point(400, 520);
            this->txtMessage->Size = Size(260, 23);

            // btnSend
            this->btnSend->Location = Point(670, 519);
            this->btnSend->Size = Size(100, 23);
            this->btnSend->Text = "Send";
            this->btnSend->Click += gcnew EventHandler(this, &FormMain::BtnSend_Click);

            // btnSurrender
            this->btnSurrender->Location = Point(670, 550);
            this->btnSurrender->Size = Size(100, 23);
            this->btnSurrender->Text = "Đầu hàng";
            this->btnSurrender->Click += gcnew EventHandler(this, &FormMain::BtnSurrender_Click);

            // FormMain
            this->AutoScaleDimensions = SizeF(7, 15);
            this->AutoScaleMode = AutoScaleMode::Font;
            this->ClientSize = Size(800, 600);
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

        // Skeleton event handlers
        void BtnConnect_Click(Object^ sender, EventArgs^ e) {}
        void BtnSend_Click(Object^ sender, EventArgs^ e) {}
        void BtnSurrender_Click(Object^ sender, EventArgs^ e) {}
    };
}}
