#ifndef MWGUI_COUNTDIALOG_H
#define MWGUI_COUNTDIALOG_H

#include "windowbase.hpp"

#include <components/widgets/scrollbar.hpp>

namespace Gui
{
    class NumericEditBox;
}

namespace MWGui
{
    class CountDialog : public WindowModal
    {
    public:
        CountDialog();
        void openCountDialog(const std::string& item, const std::string& message, const int maxCount);
        void setCount(int count);

        typedef MyGUI::delegates::MultiDelegate<MyGUI::Widget*, int> EventHandle_WidgetInt;

        /** Event : Ok button was clicked.\n
            signature : void method(MyGUI::Widget* _sender, int _count)\n
        */
        EventHandle_WidgetInt eventOkClicked;

    private:
        Gui::ScrollBar* mSlider;
        Gui::NumericEditBox* mItemEdit;
        MyGUI::TextBox* mItemText;
        MyGUI::TextBox* mLabelText;
        MyGUI::Button* mOkButton;
        MyGUI::Button* mCancelButton;

        void onCancelButtonClicked(MyGUI::Widget* _sender);
        void onOkButtonClicked(MyGUI::Widget* _sender);
        void onEditValueChanged(int value);
        void onSliderMoved(MyGUI::ScrollBar* _sender, size_t _position);
        void onEnterKeyPressed(MyGUI::EditBox* _sender);
        bool onControllerButtonEvent(const SDL_ControllerButtonEvent& arg) override;
    };

}

#endif
