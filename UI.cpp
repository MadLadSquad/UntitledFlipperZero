#include "UI.hpp"
#include <Scenes/Scenes.hpp> // TODO: Remove

// =====================================================================================================================
// ======================================================= Views =======================================================
// =====================================================================================================================

UFZ::View::View(::View* v) noexcept
{
    view = v;
}

UFZ::View::operator ::View*() noexcept
{
    return view;
}

UFZ::View& UFZ::View::allocate() noexcept
{
    view = view_alloc();
    bAllocated = true;
    return *this;
}

void UFZ::View::free() noexcept
{
    if (view != nullptr && bAllocated)
        view_free(view);
    view = nullptr;
    bAllocated = false;
}

UFZ::View::~View() noexcept
{
    free();
}

const UFZ::View& UFZ::View::setDrawCallback(ViewDrawCallback callback) const noexcept
{
    view_set_draw_callback(view, callback);
    return *this;
}

const UFZ::View& UFZ::View::setInputCallback(ViewInputCallback callback) const noexcept
{
    view_set_input_callback(view, callback);
    return *this;
}

const UFZ::View& UFZ::View::setCustomCallback(ViewCustomCallback callback) const noexcept
{
    view_set_custom_callback(view, callback);
    return *this;
}

const UFZ::View& UFZ::View::setPreviousCallback(ViewNavigationCallback callback) const noexcept
{
    view_set_previous_callback(view, callback);
    return *this;
}

const UFZ::View& UFZ::View::setEnterCallback(ViewCallback callback) const noexcept
{
    view_set_enter_callback(view, callback);
    return *this;
}

const UFZ::View& UFZ::View::setExitCallback(ViewCallback callback) const noexcept
{
    view_set_exit_callback(view, callback);
    return *this;
}

const UFZ::View& UFZ::View::setUpdateCallback(ViewUpdateCallback callback) const noexcept
{
    view_set_update_callback(view, callback);
    return *this;
}

const UFZ::View &UFZ::View::setUpdateCallbackContext(void* context) const noexcept
{
    view_set_update_callback_context(view, context);
    return *this;
}

const UFZ::View& UFZ::View::setContext(void* context) const noexcept
{
    view_set_context(view, context);
    return *this;
}

const UFZ::View& UFZ::View::setOrientation(ViewOrientation orientation) const noexcept
{
    view_set_orientation(view, orientation);
    return *this;
}

const UFZ::View& UFZ::View::allocateModel(ViewModelType type, size_t size) const noexcept
{
    view_allocate_model(view, type, size);
    return *this;
}

const UFZ::View& UFZ::View::freeModel() const noexcept
{
    view_free_model(view);
    return *this;
}

void* UFZ::View::getModel() const noexcept
{
    return view_get_model(view);
}

const UFZ::View& UFZ::View::commitModel(bool bUpdate) const noexcept
{
    view_commit_model(view, bUpdate);
    return *this;
}

void UFZ::View::setDeferredSetupCallback(const std::function<void(View&)>& f) noexcept
{
    deferredSetupCallback = f;
}

// =====================================================================================================================
// =================================================== Generic Widget ==================================================
// =====================================================================================================================

void UFZ::UWidget::addView(const UFZ::View& view) noexcept
{
    view_stack_add_view(viewStack, view.view);
}

void UFZ::UWidget::removeView(const View& view) noexcept
{
    view_stack_remove_view(viewStack, view.view);
}

UFZ::View UFZ::UWidget::getView() noexcept
{
    return UFZ::View(view_stack_get_view(viewStack));
}

void UFZ::UWidget::allocateViewStack(const View& widgetView) noexcept
{
    viewStack = view_stack_alloc();
    addView(widgetView);

    for (auto& a : views)
    {
        a->allocate();
        addView(*a);
        a->deferredSetupCallback(*a);
    }
}

void UFZ::UWidget::destroy()
{
    if (!bDestroyed)
    {
        for (auto& a : views)
        {
            removeView(*a);
            a->free();
        }
        removeView(getWidgetView());
        FREE_GUARD(view_stack_free, viewStack);
        free();
    }
    bDestroyed = true;
}

// =====================================================================================================================
// ====================================================== Widgets ======================================================
// =====================================================================================================================

const UFZ::Menu& UFZ::Menu::addItem(const char* label, const Icon* icon, uint32_t index, MenuItemCallback callback, void* context) const noexcept
{
    menu_add_item(menu, label, icon, index, callback, context);
    return *this;
}

void UFZ::Menu::setSelectedItem(uint32_t index) const noexcept
{
    menu_set_selected_item(menu, index);
}

// =====================================================================================================================
// ==================================================== Button Menu ====================================================
// =====================================================================================================================

ButtonMenuItem* UFZ::ButtonMenu::addItem(const char* label, int32_t index, ButtonMenuItemCallback callback, ButtonMenuItemType type, void* context) const noexcept
{
    return button_menu_add_item(button_menu, label, index, callback, type, context);
}

void UFZ::ButtonMenu::setHeader(const char* header) const
{
    button_menu_set_header(button_menu, header);
}

void UFZ::ButtonMenu::setSelectedItem(uint32_t index) const noexcept
{
    button_menu_set_selected_item(button_menu, index);
}

// =====================================================================================================================
// ==================================================== Button Panel ===================================================
// =====================================================================================================================

void UFZ::ButtonPanel::reserve(size_t x, size_t y) const noexcept
{
    button_panel_reserve(button_panel, x, y);
}

const UFZ::ButtonPanel& UFZ::ButtonPanel::addItem(uint32_t index,
                               uint16_t matrix_place_x, uint16_t matrix_place_y,
                               uint16_t x, uint16_t y,
                               const Icon* icon_name, const Icon* icon_name_selected,
                               ButtonItemCallback callback, void* context) const noexcept
{
    button_panel_add_item(button_panel, index, matrix_place_x, matrix_place_y, x, y, icon_name, icon_name_selected, callback, context);
    return *this;
}

const UFZ::ButtonPanel& UFZ::ButtonPanel::addLabel(uint16_t x, uint16_t y, Font font, const char* label) const noexcept
{
    button_panel_add_label(button_panel, x, y, font, label);
    return *this;
}

const UFZ::ButtonPanel& UFZ::ButtonPanel::addIcon(uint16_t x, uint16_t y, const Icon* icon) const noexcept
{
    button_panel_add_icon(button_panel, x, y, icon);
    return *this;
}

// =====================================================================================================================
// ===================================================== Byte Input ====================================================
// =====================================================================================================================

UFZ::ByteInput::~ByteInput() noexcept
{
    free();
}

void UFZ::ByteInput::setResultCallback(ByteInputCallback inputCallback, ByteChangedCallback changedCallback, void* context, uint8_t* bytes, uint8_t bytesCount) noexcept
{
    byte_input_set_result_callback(byte_input, inputCallback, changedCallback, context, bytes, bytesCount);
}

void UFZ::ByteInput::setHeaderText(const char* text) noexcept
{
    byte_input_set_header_text(byte_input, text);
}

UFZ::View UFZ::ByteInput::getWidgetView() noexcept
{
    return UFZ::View(byte_input_get_view(byte_input));
}

void UFZ::ByteInput::alloc() noexcept
{
    byte_input = byte_input_alloc();
}

void UFZ::ByteInput::free() noexcept
{
    FREE_GUARD(byte_input_free, byte_input);
}

// =====================================================================================================================
// ===================================================== Dialog Ex =====================================================
// =====================================================================================================================

const UFZ::DialogEx& UFZ::DialogEx::setResultCallback(DialogExResultCallback callback) const noexcept
{
    dialog_ex_set_result_callback(dialog_ex, callback);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setContext(void* context) const noexcept
{
    dialog_ex_set_context(dialog_ex, context);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setHeader(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) const noexcept
{
    dialog_ex_set_header(dialog_ex, text, x, y, horizontal, vertical);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setText(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) const noexcept
{
    dialog_ex_set_text(dialog_ex, text, x, y, horizontal, vertical);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setIcon(uint8_t x, uint8_t y, const Icon* icon) const noexcept
{
    dialog_ex_set_icon(dialog_ex, x, y, icon);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setLeftButtonText(const char* text) const noexcept
{
    dialog_ex_set_left_button_text(dialog_ex, text);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setCenterButtonText(const char* text) const noexcept
{
    dialog_ex_set_center_button_text(dialog_ex, text);
    return *this;
}

const UFZ::DialogEx& UFZ::DialogEx::setRightButtonText(const char* text) const noexcept
{
    dialog_ex_set_right_button_text(dialog_ex, text);
    return *this;
}

void UFZ::DialogEx::enableExtendedEvents() const noexcept
{
    dialog_ex_enable_extended_events(dialog_ex);
}

void UFZ::DialogEx::disableExtendedEvents() const noexcept
{
    dialog_ex_disable_extended_events(dialog_ex);
}

// =====================================================================================================================
// ==================================================== Empty Screen ===================================================
// =====================================================================================================================

UFZ::EmptyScreen::~EmptyScreen() noexcept
{
    free();
}

void UFZ::EmptyScreen::free() noexcept
{
    FREE_GUARD(empty_screen_free, empty_screen);
}

void UFZ::EmptyScreen::alloc() noexcept
{
    empty_screen_alloc();
}

UFZ::View UFZ::EmptyScreen::getWidgetView() noexcept
{
    return UFZ::View(empty_screen_get_view(empty_screen));
}

// =====================================================================================================================
// ====================================================== Loading ======================================================
// =====================================================================================================================

UFZ::Loading::~Loading() noexcept
{
    free();
}

void UFZ::Loading::free() noexcept
{
    FREE_GUARD(loading_free, loading);
}

void UFZ::Loading::alloc() noexcept
{
    loading = loading_alloc();
}

UFZ::View UFZ::Loading::getWidgetView() noexcept
{
    return UFZ::View(loading_get_view(loading));
}

// =====================================================================================================================
// ======================================================= Popup =======================================================
// =====================================================================================================================

const UFZ::Popup& UFZ::Popup::setCallback(PopupCallback callback) const noexcept
{
    popup_set_callback(popup, callback);
    return *this;
}

const UFZ::Popup& UFZ::Popup::setContext(void* context) const noexcept
{
    popup_set_context(popup, context);
    return *this;
}

const UFZ::Popup& UFZ::Popup::setHeader(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) const noexcept
{
    popup_set_header(popup, text, x, y, horizontal, vertical);
    return *this;
}

const UFZ::Popup& UFZ::Popup::setText(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) const noexcept
{
    popup_set_text(popup, text, x, y, horizontal, vertical);
    return *this;
}

const UFZ::Popup& UFZ::Popup::setIcon(uint8_t x, uint8_t y, const Icon* icon) const noexcept
{
    popup_set_icon(popup, x, y, icon);
    return *this;
}

const UFZ::Popup& UFZ::Popup::setTimeout(uint32_t milliseconds) const noexcept
{
    popup_set_timeout(popup, milliseconds);
    return *this;
}

void UFZ::Popup::enableTimeout() const noexcept
{
    popup_enable_timeout(popup);
}

void UFZ::Popup::disableTimout() const noexcept
{
    popup_disable_timeout(popup);
}

// =====================================================================================================================
// ====================================================== Submenu ======================================================
// =====================================================================================================================

const UFZ::Submenu& UFZ::Submenu::addItem(const char* label, uint32_t index, SubmenuItemCallback callback, void* context) const noexcept
{
    submenu_add_item(submenu, label, index, callback, context);
    return *this;
}

const UFZ::Submenu& UFZ::Submenu::setSelectedItem(uint32_t index) const noexcept
{
    submenu_set_selected_item(submenu, index);
    return *this;
}

const UFZ::Submenu& UFZ::Submenu::setHeader(const char* header) const noexcept
{
    submenu_set_header(submenu, header);
    return *this;
}

// =====================================================================================================================
// ====================================================== Textbox ======================================================
// =====================================================================================================================

const UFZ::TextBox& UFZ::TextBox::setText(const char* text) const noexcept
{
    text_box_set_text(text_box, text);
    return *this;
}

const UFZ::TextBox& UFZ::TextBox::setFont(TextBoxFont font) const noexcept
{
    text_box_set_font(text_box, font);
    return *this;
}

const UFZ::TextBox& UFZ::TextBox::setFocus(TextBoxFocus focus) const noexcept
{
    text_box_set_focus(text_box, focus);
    return *this;
}

// =====================================================================================================================
// ===================================================== Text input ====================================================
// =====================================================================================================================

void UFZ::TextInput::setResultCallback(TextInputCallback callback, void* callbackContext, char* textBuffer, size_t textBufferSize, bool clearDefaultText) const noexcept
{
    text_input_set_result_callback(text_input, callback, callbackContext, textBuffer, textBufferSize, clearDefaultText);
}

void UFZ::TextInput::setValidator(TextInputValidatorCallback callback, void* context) const noexcept
{
    text_input_set_validator(text_input, callback, context);
}

void UFZ::TextInput::getValidatorCallbackContext() const noexcept
{
    text_input_get_validator_callback_context(text_input);
}

void UFZ::TextInput::setHeaderText(const char* text) const noexcept
{
    text_input_set_header_text(text_input, text);
}

// =====================================================================================================================
// ================================================= Variable Item List  ===============================================
// =====================================================================================================================

VariableItem* UFZ::VariableItemList::add(const char* label, uint8_t values_count, VariableItemChangeCallback callback, void* context) const noexcept
{
    return variable_item_list_add(variable_item_list, label, values_count, callback, context);
}

void UFZ::VariableItemList::setEnterCallback(VariableItemListEnterCallback callback, void* context) const noexcept
{
    variable_item_list_set_enter_callback(variable_item_list, callback, context);
}

void UFZ::VariableItemList::setSelectedItem(uint8_t index) const noexcept
{
    variable_item_list_set_selected_item(variable_item_list, index);
}

uint8_t UFZ::VariableItemList::getSelectedItemIndex() const noexcept
{
    return variable_item_list_get_selected_item_index(variable_item_list);
}

// =====================================================================================================================
// ======================================================= Widget ======================================================
// =====================================================================================================================

const UFZ::Widget& UFZ::Widget::addStringMultilineElement(uint8_t x, uint8_t y, Align horizontal, Align vertical, Font font, const char* text) const noexcept
{
    widget_add_string_multiline_element(widget, x, y, horizontal, vertical, font, text);
    return *this;
}

const UFZ::Widget& UFZ::Widget::addStringElement(uint8_t x, uint8_t y, Align horizontal, Align vertical, Font font, const char* text) const noexcept
{
    widget_add_string_element(widget, x, y, horizontal, vertical, font, text);
    return *this;
}

const UFZ::Widget& UFZ::Widget::addTextBoxElement(uint8_t x, uint8_t y, uint8_t width, uint8_t height, Align horizontal, Align vertical, const char* text, bool stripToDots) const noexcept
{
    widget_add_text_box_element(widget, x, y, width, height, horizontal, vertical, text, stripToDots);
    return *this;
}

const UFZ::Widget& UFZ::Widget::addTextScrollElement(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const char* text) const noexcept
{
    widget_add_text_scroll_element(widget, x, y, width, height, text);
    return *this;
}

const UFZ::Widget& UFZ::Widget::addButtonElement(GuiButtonType type, const char* text, ButtonCallback callback, void* context) const noexcept
{
    widget_add_button_element(widget, type, text, callback, context);
    return *this;
}

const UFZ::Widget& UFZ::Widget::addIconElement(uint8_t x, uint8_t y, const Icon* icon) const noexcept
{
    widget_add_icon_element(widget, x, y, icon);
    return *this;
}

const UFZ::Widget& UFZ::Widget::addFrameElement(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t radius) const noexcept
{
    widget_add_frame_element(widget, x, y, width, height, radius);
    return *this;
}