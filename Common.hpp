#pragma once
#include <vector>
#include <functional>
#include <furi.h>
#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/view_dispatcher.h>
// Only frees if memory is allocated
#define FREE_GUARD(x, y) if ((y) != nullptr) x(y); (y) = nullptr;

namespace UFZ
{
    class UWidget;
    class Application;

    class ViewDispatcher
    {
    public:
        ViewDispatcher() = default;

        void switchToView(uint32_t id) const noexcept;
        void sendToFront() const noexcept;
        void sendToBack() const noexcept;

        ~ViewDispatcher() noexcept;
    private:
        friend class Application;

        void init() noexcept;
        void free() noexcept;

        ::ViewDispatcher* viewDispatcher = nullptr;
    };

    class SceneManager
    {
    public:
        SceneManager() = default;

        void setSceneState(uint32_t id, uint32_t state) const noexcept;
        [[nodiscard]] uint32_t getSceneState(uint32_t id) const noexcept;

        [[nodiscard]] bool handleCustomEvent(uint32_t event) const noexcept;
        [[nodiscard]] bool handleBackEvent() const noexcept;
        void handleTickEvent() const noexcept;

        void nextScene(uint32_t id) const noexcept;
        [[nodiscard]] bool previousScene() const noexcept;
        [[nodiscard]] bool hasPreviousScene(uint32_t id) const noexcept;

        [[nodiscard]] bool searchAndSwitchToPreviousScene(uint32_t id) const noexcept;
        bool searchAndSwitchToPreviousSceneOneOf(const uint32_t* ids, size_t idsSize) const noexcept;
        void searchAndSwitchToAnotherScene(uint32_t id) const noexcept;

        ~SceneManager();
    private:
        friend class Application;

        void alloc(const SceneManagerHandlers& handlers, Application& app) noexcept;
        void free() noexcept;
        void stop() const noexcept;

        ::SceneManager* sceneManager = nullptr;
        Application* application = nullptr;
    };

    class Application
    {
    public:
        Application() = default;
        explicit Application(const std::vector<UWidget*>& widgetsRef, void* userPointer, uint32_t tickPeriod = 0) noexcept;
        void init(const std::vector<UWidget*>& widgetsRef, void* userPointer, uint32_t tickPeriod = 0) noexcept;

        template<typename T>
        T* getWidget(size_t i) noexcept
        {
            return static_cast<T*>(widgets[i]);
        }

        const ViewDispatcher& getViewDispatcher() noexcept;
        const SceneManager& getSceneManager() noexcept;

        void* getUserPointer() noexcept;

        void destroy() noexcept;
        ~Application() noexcept;
    private:
        SceneManager sceneManager;
        ViewDispatcher viewDispatcher;
        Gui* gui = nullptr;

        void* ctx = nullptr;
        size_t tickInterval = 0;

        std::vector<AppSceneOnEnterCallback> enterCallbacks{};
        std::vector<AppSceneOnEventCallback> eventCallbacks{};
        std::vector<AppSceneOnExitCallback> exitCallbacks{};

        SceneManagerHandlers handlers{};

        std::vector<UWidget*> widgets;

        void initSceneManager() noexcept;
        void initViewDispatcher() noexcept;
        void initGUI() noexcept;

        void freeSceneManager() noexcept;
        void freeViewDispatcher() noexcept;
        static void freeGUI() noexcept;
    };
}