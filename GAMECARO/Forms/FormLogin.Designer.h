#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace Client {
namespace Forms {

    public ref class FormLogin : public Form
    {
    public:
        FormLogin()
        {
            InitializeComponent();
        }

    protected:
        ~FormLogin()
        {
            if (components)
            {
                delete components;
            }
        }

    private:
        IContainer^ components;

        void InitializeComponent()
        {
            this->components = gcnew Container();
            this->AutoScaleMode = AutoScaleMode::Font;
            this->ClientSize = Size(800, 450);
            this->Text = "FormLogin";
        }
    };
}}
