#include "enchantingdialog.hpp"

#include <iomanip>

#include <MyGUI_Button.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ScrollView.h>
#include <MyGUI_UString.h>

#include <components/misc/strings/format.hpp>
#include <components/settings/values.hpp>
#include <components/widgets/list.hpp>

#include <components/esm3/loadgmst.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/mechanicsmanager.hpp"
#include "../mwbase/windowmanager.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/containerstore.hpp"
#include "../mwworld/esmstore.hpp"

#include "../mwmechanics/actorutil.hpp"

#include "itemselection.hpp"
#include "itemwidget.hpp"

#include "sortfilteritemmodel.hpp"

namespace MWGui
{

    EnchantingDialog::EnchantingDialog()
        : WindowBase("openmw_enchanting_dialog.layout")
        , EffectEditorBase(EffectEditorBase::Enchanting)
    {
        getWidget(mName, "NameEdit");
        getWidget(mCancelButton, "CancelButton");
        getWidget(mAvailableEffectsList, "AvailableEffects");
        getWidget(mUsedEffectsView, "UsedEffects");
        getWidget(mItemBox, "ItemBox");
        getWidget(mSoulBox, "SoulBox");
        getWidget(mEnchantmentPoints, "Enchantment");
        getWidget(mCastCost, "CastCost");
        getWidget(mCharge, "Charge");
        getWidget(mSuccessChance, "SuccessChance");
        getWidget(mChanceLayout, "ChanceLayout");
        getWidget(mTypeButton, "TypeButton");
        getWidget(mBuyButton, "BuyButton");
        getWidget(mPrice, "PriceLabel");
        getWidget(mPriceText, "PriceTextLabel");

        setWidgets(mAvailableEffectsList, mUsedEffectsView);

        mCancelButton->eventMouseButtonClick += MyGUI::newDelegate(this, &EnchantingDialog::onCancelButtonClicked);
        mItemBox->eventMouseButtonClick += MyGUI::newDelegate(this, &EnchantingDialog::onSelectItem);
        mSoulBox->eventMouseButtonClick += MyGUI::newDelegate(this, &EnchantingDialog::onSelectSoul);
        mBuyButton->eventMouseButtonClick += MyGUI::newDelegate(this, &EnchantingDialog::onBuyButtonClicked);
        mTypeButton->eventMouseButtonClick += MyGUI::newDelegate(this, &EnchantingDialog::onTypeButtonClicked);
        mName->eventEditSelectAccept += MyGUI::newDelegate(this, &EnchantingDialog::onAccept);

        mControllerButtons.a = "#{sSelect}";
        mControllerButtons.b = "#{sCancel}";
        mControllerButtons.y = "Cast Type";
        mControllerButtons.l1 = "#{sItem}";
        mControllerButtons.r1 = "#{sSoulGem}";
    }

    void EnchantingDialog::onOpen()
    {
        center();
        MWBase::Environment::get().getWindowManager()->setKeyFocusWidget(mName);
    }

    void EnchantingDialog::setSoulGem(const MWWorld::Ptr& gem)
    {
        if (gem.isEmpty())
        {
            mSoulBox->setItem(MWWorld::Ptr());
            mSoulBox->clearUserStrings();
            mEnchanting.setSoulGem(MWWorld::Ptr());
        }
        else
        {
            mSoulBox->setItem(gem);
            mSoulBox->setUserString("ToolTipType", "ItemPtr");
            mSoulBox->setUserData(MWWorld::Ptr(gem));
            mEnchanting.setSoulGem(gem);
        }
    }

    void EnchantingDialog::setItem(const MWWorld::Ptr& item)
    {
        if (item.isEmpty())
        {
            mItemBox->setItem(MWWorld::Ptr());
            mItemBox->clearUserStrings();
            mEnchanting.setOldItem(MWWorld::Ptr());
        }
        else
        {
            std::string_view name = item.getClass().getName(item);
            mName->setCaption(MyGUI::UString(name));
            mItemBox->setItem(item);
            mItemBox->setUserString("ToolTipType", "ItemPtr");
            mItemBox->setUserData(MWWorld::Ptr(item));
            mEnchanting.setOldItem(item);
        }
    }

    void EnchantingDialog::updateLabels()
    {
        mEnchantmentPoints->setCaption(std::to_string(static_cast<int>(mEnchanting.getEnchantPoints(false))) + " / "
            + std::to_string(mEnchanting.getMaxEnchantValue()));
        mCharge->setCaption(std::to_string(mEnchanting.getGemCharge()));
        mSuccessChance->setCaption(std::to_string(std::clamp(mEnchanting.getEnchantChance(), 0, 100)));
        mCastCost->setCaption(std::to_string(mEnchanting.getEffectiveCastCost()));
        mPrice->setCaption(std::to_string(mEnchanting.getEnchantPrice()));

        switch (mEnchanting.getCastStyle())
        {
            case ESM::Enchantment::CastOnce:
                mTypeButton->setCaption(MyGUI::UString(
                    MWBase::Environment::get().getWindowManager()->getGameSettingString("sItemCastOnce", "Cast Once")));
                setConstantEffect(false);
                break;
            case ESM::Enchantment::WhenStrikes:
                mTypeButton->setCaption(
                    MyGUI::UString(MWBase::Environment::get().getWindowManager()->getGameSettingString(
                        "sItemCastWhenStrikes", "When Strikes")));
                setConstantEffect(false);
                break;
            case ESM::Enchantment::WhenUsed:
                mTypeButton->setCaption(
                    MyGUI::UString(MWBase::Environment::get().getWindowManager()->getGameSettingString(
                        "sItemCastWhenUsed", "When Used")));
                setConstantEffect(false);
                break;
            case ESM::Enchantment::ConstantEffect:
                mTypeButton->setCaption(
                    MyGUI::UString(MWBase::Environment::get().getWindowManager()->getGameSettingString(
                        "sItemCastConstant", "Cast Constant")));
                setConstantEffect(true);
                break;
        }
    }

    void EnchantingDialog::setPtr(const MWWorld::Ptr& ptr)
    {
        if (ptr.isEmpty() || (ptr.getType() != ESM::REC_MISC && !ptr.getClass().isActor()))
            throw std::runtime_error("Invalid argument in EnchantingDialog::setPtr");

        mName->setCaption({});

        if (ptr.getClass().isActor())
        {
            mEnchanting.setSelfEnchanting(false);
            mEnchanting.setEnchanter(ptr);
            mBuyButton->setCaptionWithReplacing("#{sBuy}");
            mControllerButtons.x = "#{sBuy}";
            mChanceLayout->setVisible(false);
            mPtr = ptr;
            setSoulGem(MWWorld::Ptr());
            mPrice->setVisible(true);
            mPriceText->setVisible(true);
        }
        else
        {
            mEnchanting.setSelfEnchanting(true);
            mEnchanting.setEnchanter(MWMechanics::getPlayer());
            mBuyButton->setCaptionWithReplacing("#{sCreate}");
            mControllerButtons.x = "#{sCreate}";
            mChanceLayout->setVisible(Settings::game().mShowEnchantChance);
            mPtr = MWMechanics::getPlayer();
            setSoulGem(ptr);
            mPrice->setVisible(false);
            mPriceText->setVisible(false);
        }

        setItem(MWWorld::Ptr());
        startEditing();
        updateLabels();
    }

    void EnchantingDialog::onReferenceUnavailable()
    {
        MWBase::Environment::get().getWindowManager()->removeGuiMode(GM_Enchanting);
        resetReference();
    }

    void EnchantingDialog::resetReference()
    {
        ReferenceInterface::resetReference();
        setItem(MWWorld::Ptr());
        setSoulGem(MWWorld::Ptr());
        mPtr = MWWorld::Ptr();
        mEnchanting.setEnchanter(MWWorld::Ptr());
    }

    void EnchantingDialog::onCancelButtonClicked(MyGUI::Widget* sender)
    {
        MWBase::Environment::get().getWindowManager()->removeGuiMode(GM_Enchanting);
    }

    void EnchantingDialog::onSelectItem(MyGUI::Widget* sender)
    {
        if (mEnchanting.getOldItem().isEmpty())
        {
            mItemSelectionDialog = std::make_unique<ItemSelectionDialog>("#{sEnchantItems}");
            mItemSelectionDialog->eventItemSelected += MyGUI::newDelegate(this, &EnchantingDialog::onItemSelected);
            mItemSelectionDialog->eventDialogCanceled += MyGUI::newDelegate(this, &EnchantingDialog::onItemCancel);
            mItemSelectionDialog->setVisible(true);
            mItemSelectionDialog->openContainer(MWMechanics::getPlayer());
            mItemSelectionDialog->setFilter(SortFilterItemModel::Filter_OnlyEnchantable);
        }
        else
        {
            setItem(MWWorld::Ptr());
            updateLabels();
        }
    }

    void EnchantingDialog::onItemSelected(MWWorld::Ptr item)
    {
        mItemSelectionDialog->setVisible(false);

        setItem(item);
        MWBase::Environment::get().getWindowManager()->playSound(item.getClass().getDownSoundId(item));
        mEnchanting.nextCastStyle();
        updateLabels();
    }

    void EnchantingDialog::onItemCancel()
    {
        mItemSelectionDialog->setVisible(false);
    }

    void EnchantingDialog::onSoulSelected(MWWorld::Ptr item)
    {
        mItemSelectionDialog->setVisible(false);

        mEnchanting.setSoulGem(item);
        if (mEnchanting.getGemCharge() == 0)
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage32}");
            return;
        }

        setSoulGem(item);
        MWBase::Environment::get().getWindowManager()->playSound(item.getClass().getDownSoundId(item));
        updateLabels();
    }

    void EnchantingDialog::onSoulCancel()
    {
        mItemSelectionDialog->setVisible(false);
    }

    void EnchantingDialog::onSelectSoul(MyGUI::Widget* sender)
    {
        if (mEnchanting.getGem().isEmpty())
        {
            mItemSelectionDialog = std::make_unique<ItemSelectionDialog>("#{sSoulGemsWithSouls}");
            mItemSelectionDialog->eventItemSelected += MyGUI::newDelegate(this, &EnchantingDialog::onSoulSelected);
            mItemSelectionDialog->eventDialogCanceled += MyGUI::newDelegate(this, &EnchantingDialog::onSoulCancel);
            mItemSelectionDialog->setVisible(true);
            mItemSelectionDialog->openContainer(MWMechanics::getPlayer());
            mItemSelectionDialog->setFilter(SortFilterItemModel::Filter_OnlyChargedSoulstones);

            // MWBase::Environment::get().getWindowManager()->messageBox("#{sInventorySelectNoSoul}");
        }
        else
        {
            setSoulGem(MWWorld::Ptr());
            mEnchanting.nextCastStyle();
            updateLabels();
            updateEffectsView();
        }
    }

    void EnchantingDialog::notifyEffectsChanged()
    {
        mEffectList.populate(mEffects);
        mEnchanting.setEffect(mEffectList);
        updateLabels();
    }

    void EnchantingDialog::onTypeButtonClicked(MyGUI::Widget* sender)
    {
        mEnchanting.nextCastStyle();
        updateLabels();
        updateEffectsView();
    }

    void EnchantingDialog::onAccept(MyGUI::EditBox* sender)
    {
        onBuyButtonClicked(sender);

        // To do not spam onAccept() again and again
        MWBase::Environment::get().getWindowManager()->injectKeyRelease(MyGUI::KeyCode::None);
    }

    void EnchantingDialog::onBuyButtonClicked(MyGUI::Widget* sender)
    {
        if (mEffects.size() <= 0)
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sEnchantmentMenu11}");
            return;
        }

        if (mName->getCaption().empty())
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage10}");
            return;
        }

        if (mEnchanting.soulEmpty())
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage52}");
            return;
        }

        if (mEnchanting.itemEmpty())
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage11}");
            return;
        }

        if (static_cast<int>(mEnchanting.getEnchantPoints(false)) > mEnchanting.getMaxEnchantValue())
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage29}");
            return;
        }

        mEnchanting.setNewItemName(mName->getCaption());
        mEnchanting.setEffect(mEffectList);

        MWWorld::Ptr player = MWMechanics::getPlayer();
        int playerGold = player.getClass().getContainerStore(player).count(MWWorld::ContainerStore::sGoldId);
        if (mPtr != player && mEnchanting.getEnchantPrice() > playerGold)
        {
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage18}");
            return;
        }

        // check if the player is attempting to use a soulstone or item that was stolen from this actor
        if (mPtr != player)
        {
            for (int i = 0; i < 2; ++i)
            {
                MWWorld::Ptr item = (i == 0) ? mEnchanting.getOldItem() : mEnchanting.getGem();
                if (MWBase::Environment::get().getMechanicsManager()->isItemStolenFrom(
                        item.getCellRef().getRefId(), mPtr))
                {
                    std::string msg = MWBase::Environment::get()
                                          .getESMStore()
                                          ->get<ESM::GameSetting>()
                                          .find("sNotifyMessage49")
                                          ->mValue.getString();
                    msg = Misc::StringUtils::format(msg, item.getClass().getName(item));
                    MWBase::Environment::get().getWindowManager()->messageBox(msg);

                    MWBase::Environment::get().getMechanicsManager()->confiscateStolenItemToOwner(
                        player, item, mPtr, 1);

                    MWBase::Environment::get().getWindowManager()->removeGuiMode(GM_Enchanting);
                    MWBase::Environment::get().getWindowManager()->exitCurrentGuiMode();
                    return;
                }
            }
        }

        if (mEnchanting.create())
        {
            MWBase::Environment::get().getWindowManager()->playSound(ESM::RefId::stringRefId("enchant success"));
            MWBase::Environment::get().getWindowManager()->messageBox("#{sEnchantmentMenu12}");
            MWBase::Environment::get().getWindowManager()->removeGuiMode(GM_Enchanting);
        }
        else
        {
            MWBase::Environment::get().getWindowManager()->playSound(ESM::RefId::stringRefId("enchant fail"));
            MWBase::Environment::get().getWindowManager()->messageBox("#{sNotifyMessage34}");
            if (!mEnchanting.getGem().isEmpty() && !mEnchanting.getGem().getCellRef().getCount())
            {
                setSoulGem(MWWorld::Ptr());
                mEnchanting.nextCastStyle();
                updateLabels();
                updateEffectsView();
            }
        }
    }

    bool EnchantingDialog::onControllerButtonEvent(const SDL_ControllerButtonEvent& arg)
    {
        if (arg.button == SDL_CONTROLLER_BUTTON_B)
            onCancelButtonClicked(mCancelButton);
        else if (arg.button == SDL_CONTROLLER_BUTTON_X)
            onBuyButtonClicked(mBuyButton);
        else if (arg.button == SDL_CONTROLLER_BUTTON_Y)
            onTypeButtonClicked(mTypeButton);
        else if (arg.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
            onSelectItem(mItemBox);
        else if (arg.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
            onSelectSoul(mSoulBox);
        else
            return EffectEditorBase::onControllerButtonEvent(arg);

        return true;
    }
}
