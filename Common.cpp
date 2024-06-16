#include "Common.hpp"
#include "UI.hpp"

UFZ::Application::Application(const std::vector<UWidget*>& widgetsRef, void* userPointer, uint32_t tickPeriod) noexcept
{
    init(widgetsRef, userPointer, tickPeriod);
}

void UFZ::Application::init(const std::vector<UWidget*>& widgetsRef, void* userPointer, uint32_t tickPeriod) noexcept
{
    widgets = widgetsRef;
    tickInterval = tickPeriod;
    ctx = userPointer;

    uint32_t size = widgets.size();
    enterCallbacks.reserve(size);
    eventCallbacks.reserve(size);
    exitCallbacks.reserve(size);

    for (auto& a : widgets)
    {
        enterCallbacks.push_back(a->enter);
        eventCallbacks.push_back(a->event);
        exitCallbacks.push_back(a->exit);
    }

    handlers.on_enter_handlers = enterCallbacks.data();
    handlers.on_event_handlers = eventCallbacks.data();
    handlers.on_exit_handlers = exitCallbacks.data();
    memcpy((void*)&handlers.scene_num, (void*)&size, sizeof(uint32_t));

    initSceneManager();
    initViewDispatcher();
    initGUI();
    filesystem.init();

    sceneManager.nextScene(0);
    view_dispatcher_run(viewDispatcher.viewDispatcher);
}

void UFZ::Application::initSceneManager() noexcept
{
    sceneManager.alloc(handlers, *this);
}

void UFZ::Application::initViewDispatcher() noexcept
{
    viewDispatcher.init();

    for (size_t i = 0; i < widgets.size(); i++)
    {
        auto& a = widgets[i];
        a->application = this;
        a->id = i;
        a->alloc();
        a->allocateViewStack(a->getWidgetView());
        view_dispatcher_add_view(viewDispatcher.viewDispatcher, i, a->getView());
    }

    view_dispatcher_set_event_callback_context(viewDispatcher.viewDispatcher, this);
    view_dispatcher_set_custom_event_callback(viewDispatcher.viewDispatcher, [](void* context, uint32_t customEvent) -> bool {
        furi_assert(context);
        UNUSED(static_cast<Application*>(context)->sceneManager.handleCustomEvent(customEvent));
        return true;
    });
    view_dispatcher_set_navigation_event_callback(viewDispatcher.viewDispatcher, [](void* context) -> bool {
        furi_assert(context);
        UNUSED(static_cast<Application*>(context)->sceneManager.handleBackEvent());
        return true;
    });

    if (tickInterval > 0)
    {
        view_dispatcher_set_tick_event_callback(viewDispatcher.viewDispatcher, [](void* context) -> void {
            furi_assert(context);
            static_cast<Application*>(context)->sceneManager.handleTickEvent();
        }, tickInterval);
    }
}

void UFZ::Application::initGUI() noexcept
{
    gui = (Gui*)furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(viewDispatcher.viewDispatcher, gui, ViewDispatcherTypeFullscreen);
}

void UFZ::Application::destroy() noexcept
{
    if (!bDestroyed)
    {
        freeSceneManager();
        freeViewDispatcher();
        freeGUI();
        filesystem.destroy();
    }
    bDestroyed = true;
}

void UFZ::Application::freeSceneManager() noexcept
{
    sceneManager.free();
}

void UFZ::Application::freeViewDispatcher() noexcept
{
    for (size_t i = 0; i < widgets.size(); i++)
        view_dispatcher_remove_view(viewDispatcher.viewDispatcher, i);
    viewDispatcher.free();
}

void UFZ::Application::freeGUI() noexcept
{
    furi_record_close(RECORD_GUI);
}

const UFZ::ViewDispatcher& UFZ::Application::getViewDispatcher() noexcept
{
    return viewDispatcher;
}

const UFZ::SceneManager& UFZ::Application::getSceneManager() noexcept
{
    return sceneManager;
}

void* UFZ::Application::getUserPointer() noexcept
{
    return ctx;
}

const UFZ::Filesystem& UFZ::Application::getFilesystem() noexcept
{
    return filesystem;
}

// =====================================================================================================================
// ================================================== View dispatcher ==================================================
// =====================================================================================================================

void UFZ::ViewDispatcher::init() noexcept
{
    viewDispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(viewDispatcher);
}

void UFZ::ViewDispatcher::free() noexcept
{
    FREE_GUARD(view_dispatcher_free, viewDispatcher);
}

void UFZ::ViewDispatcher::switchToView(uint32_t id) const noexcept
{
    view_dispatcher_switch_to_view(viewDispatcher, id);
}

void UFZ::ViewDispatcher::sendToFront() const noexcept
{
    view_dispatcher_send_to_front(viewDispatcher);
}

void UFZ::ViewDispatcher::sendToBack() const noexcept
{
    view_dispatcher_send_to_back(viewDispatcher);
}

UFZ::ViewDispatcher::~ViewDispatcher() noexcept
{
    free();
}

// =====================================================================================================================
// =================================================== Scene manager ===================================================
// =====================================================================================================================

void UFZ::SceneManager::setSceneState(uint32_t id, uint32_t state) const noexcept
{
    scene_manager_set_scene_state(sceneManager, id, state);
}

uint32_t UFZ::SceneManager::getSceneState(uint32_t id) const noexcept
{
    return scene_manager_get_scene_state(sceneManager, id);
}

bool UFZ::SceneManager::handleCustomEvent(uint32_t event) const noexcept
{
    return scene_manager_handle_custom_event(sceneManager, event);
}

bool UFZ::SceneManager::handleBackEvent() const noexcept
{
    return scene_manager_handle_back_event(sceneManager);
}

void UFZ::SceneManager::handleTickEvent() const noexcept
{
    scene_manager_handle_tick_event(sceneManager);
}

void UFZ::SceneManager::nextScene(uint32_t id) const noexcept
{
    return scene_manager_next_scene(sceneManager, id);
}

bool UFZ::SceneManager::previousScene() const noexcept
{
    return scene_manager_previous_scene(sceneManager);
}

bool UFZ::SceneManager::hasPreviousScene(uint32_t id) const noexcept
{
    return scene_manager_has_previous_scene(sceneManager, id);
}

bool UFZ::SceneManager::searchAndSwitchToPreviousScene(uint32_t id) const noexcept
{
    return scene_manager_search_and_switch_to_previous_scene(sceneManager, id);
}

bool UFZ::SceneManager::searchAndSwitchToPreviousSceneOneOf(const uint32_t* ids, size_t idsSize) const noexcept
{
    return scene_manager_search_and_switch_to_previous_scene_one_of(sceneManager, ids, idsSize);
}

bool UFZ::SceneManager::searchAndSwitchToAnotherScene(uint32_t id) const noexcept
{
    return scene_manager_search_and_switch_to_another_scene(sceneManager, id);
}

void UFZ::SceneManager::stop() const noexcept
{
    scene_manager_stop(sceneManager);
}

UFZ::SceneManager::~SceneManager()
{
    free();
}

void UFZ::SceneManager::alloc(const SceneManagerHandlers& handlers, Application& app) noexcept
{
    application = &app;
    sceneManager = scene_manager_alloc(&handlers, application);
}

void UFZ::SceneManager::free() noexcept
{
    FREE_GUARD(scene_manager_free, sceneManager);
}