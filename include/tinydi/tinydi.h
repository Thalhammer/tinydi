#pragma once
static_assert(__cplusplus >= 201103L, "This library requires C++11 or higher");

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include <vector>

namespace tinydi
{
    class dependency_not_found_exception : public std::logic_error
    {
    public:
        dependency_not_found_exception(const std::type_info &info)
            : std::logic_error(std::string("Could not find dependency ") + info.name() + " in injector")
        {
        }
        dependency_not_found_exception(const std::string &msg)
            : std::logic_error(msg)
        {
        }
    };

    /**
     * \brief Main Injector class
     */
    class injector
    {
        mutable std::recursive_mutex m_mtx;
        std::function<void(std::exception_ptr)> m_except_handler;
        struct info
        {
            using builder_fn_t = std::function<std::shared_ptr<void>(const tinydi::injector &)>;
            std::vector<std::pair<std::shared_ptr<void>, builder_fn_t>> instances = {};
            bool in_building = false;
        };
        mutable std::unordered_map<std::type_index, info> m_types;

    public:
        /**
         * \brief Get service of type T
         * 
         * If the service was not yet created it will try instanciate it using the builder defined at the bind statement.
         * If it can't create a valid instance for whatever reason it will return nullptr and the exception handler is called
         * with any error happening during creation.
         * \tparam T Type of the service interface
         * \return Returns instance of service or nullptr
         */
        template <typename T>
        typename std::enable_if<std::is_polymorphic<T>::value, std::shared_ptr<T>>::type get(std::nothrow_t) const noexcept
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            auto it = m_types.find(typeid(T));
            // Type not registered
            if (it == m_types.cend() || it->second.instances.empty())
                return nullptr;
            auto it2 = it->second.instances.begin();
            if (it2->first != nullptr)
                return std::static_pointer_cast<T>(it2->first);
            if (it->second.in_building)
                return nullptr; // Recursive
            it->second.in_building = true;
            for (auto &e : it->second.instances)
            {
                try
                {
                    auto inst = e.second(*this);
                    it->second.in_building = false;
                    if (!inst)
                        return nullptr;
                    e.first = inst;
                    return std::static_pointer_cast<T>(inst);
                }
                catch (...)
                {
                    if (m_except_handler)
                    {
                        m_except_handler(std::current_exception());
                    }
                }
            }
            it->second.in_building = false;
            return nullptr;
        }

        /**
         * \brief Get service of type T
         * 
         * Similar to the nothrow overload, but throws if it can't create a valid instance.
         * Always returns a non nullptr pointer.
         * \tparam T Type of the service interface
         * \return Returns instance of service
         */
        template <typename T>
        typename std::enable_if<std::is_polymorphic<T>::value, std::shared_ptr<T>>::type get() const
        {
            auto res = get<T>(std::nothrow);
            if (!res)
                throw dependency_not_found_exception(typeid(T));
            return res;
        }

        /**
         * \brief Get all service instances of type T
         * 
         * Returns all services registered for the given interface. If one of the service can not be created
         * the service will be skiped and not be part of the returned list. The exception handler will be called
         * with the exception. If there is no service bound to the given interface or the creation of all services fails
         * it returns an empty list.
         * \tparam T Type of the service interface
         * \return Returns vector of services
         */
        template <typename T>
        typename std::enable_if<std::is_polymorphic<T>::value, std::vector<std::shared_ptr<T>>>::type get_all() const noexcept
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            auto it = m_types.find(typeid(T));
            // Type not registered
            if (it == m_types.cend() || it->second.instances.empty())
                return {};
            if (it->second.in_building)
                return {}; // Recursive
            it->second.in_building = true;
            std::vector<std::shared_ptr<T>> res;
            for (auto &e : it->second.instances)
            {
                try
                {
                    if (!e.first)
                    {
                        auto inst = e.second(*this);
                        if (!inst)
                            continue;
                        e.first = inst;
                    }
                    res.push_back(std::static_pointer_cast<T>(e.first));
                }
                catch (...)
                {
                    if (m_except_handler)
                    {
                        m_except_handler(std::current_exception());
                    }
                }
            }
            it->second.in_building = false;
            return res;
        }

        /**
         * \brief Bind a builder function to the given interface.
         * 
         * Binds a builder function which is called to get an instance of the given service.
         * Returning nullptr is considered an error and the result will not be part of any returned services.
         * 
         * \tparam TInterface Type of the service interface
         * \param fn Function to be called for service creation
         */
        template <typename TInterface, typename = typename std::enable_if<std::is_polymorphic<TInterface>::value, void>::type>
        void bind(std::function<std::shared_ptr<TInterface>(const tinydi::injector &)> fn)
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            auto it = m_types.find(typeid(TInterface));
            if (it != m_types.end())
            {
                it->second.instances.emplace_back(nullptr, [fn = std::move(fn)](const tinydi::injector &i) { return fn(i); });
            }
            else
            {
                info e = {};
                e.instances.emplace_back(nullptr, [fn = std::move(fn)](const tinydi::injector &i) { return fn(i); });
                m_types.emplace(typeid(TInterface), std::move(e));
            }
        }

        /**
         * \brief Bind a service implementation class to the given interface.
         * 
         * Binds an implementation of the given interface. Calls make_shared with the implementation class
         * when it gets built. Registeres both TImpl and TInterface with the injector allowing you to directly
         * get the implementation type without casting if needed.
         * 
         * \tparam TInterface Type of the service interface
         * \tparam TImpl Type of the service implementation
         */
        template <typename TInterface, typename TImpl,
                  typename = typename std::enable_if<std::is_polymorphic<TInterface>::value>::type,
                  typename = typename std::enable_if<std::is_base_of<TInterface, TImpl>::value>::type>
        void bind()
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            auto it = m_types.find(typeid(TImpl));
            if (it != m_types.end())
            {
                it->second.instances.emplace_back(nullptr, [](const tinydi::injector &) { return std::make_shared<TImpl>(); });
            }
            else
            {
                info e = {};
                e.instances.emplace_back(nullptr, [](const tinydi::injector &) { return std::make_shared<TImpl>(); });
                m_types.emplace(typeid(TImpl), std::move(e));
            }
            if (!std::is_same<TInterface, TImpl>::value)
            {
                it = m_types.find(typeid(TInterface));
                if (it != m_types.end())
                {
                    it->second.instances.emplace_back(nullptr, [](const tinydi::injector &i) { return i.get<TImpl>(); });
                }
                else
                {
                    info e = {};
                    e.instances.emplace_back(nullptr, [](const tinydi::injector &i) { return i.get<TImpl>(); });
                    m_types.emplace(typeid(TInterface), std::move(e));
                }
            }
        }

        /**
         * \brief Bind a builder function to the given interface, removing all existing implementations.
         * 
         * Binds a builder function which is called to get an instance of the given service.
         * Returning nullptr is considered an error and the result will not be part of any returned services.
         * All prior bindings are removed.
         * 
         * \tparam TInterface Type of the service interface
         * \param fn Function to be called for service creation
         */
        template <typename TInterface, typename = typename std::enable_if<std::is_polymorphic<TInterface>::value, void>::type>
        void replace(std::function<std::shared_ptr<TInterface>(const tinydi::injector &)> fn)
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            m_types.erase(typeid(TInterface));
            info e = {};
            e.instances.emplace_back(nullptr, [fn = std::move(fn)](const tinydi::injector &i) { return fn(i); });
            m_types.emplace(typeid(TInterface), std::move(e));
        }

        /**
         * \brief Bind a service implementation class to the given interface, removing all existing implementations.
         * 
         * Binds an implementation of the given interface. Calls make_shared with the implementation class
         * when it gets built. Registeres both TImpl and TInterface with the injector allowing you to directly
         * get the implementation type without casting if needed. All prior bindings are removed.
         * 
         * \tparam TInterface Type of the service interface
         * \tparam TImpl Type of the service implementation
         */
        template <typename TInterface, typename TImpl,
                  typename = typename std::enable_if<std::is_polymorphic<TInterface>::value>::type,
                  typename = typename std::enable_if<std::is_base_of<TInterface, TImpl>::value>::type>
        void replace()
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            m_types.erase(typeid(TImpl));
            m_types.erase(typeid(TInterface));
            info e = {};
            e.instances.emplace_back(nullptr, [](const tinydi::injector &) { return std::make_shared<TImpl>(); });
            m_types.emplace(typeid(TImpl), std::move(e));
            if (!std::is_same<TInterface, TImpl>::value)
            {
                info e = {};
                e.instances.emplace_back(nullptr, [](const tinydi::injector &i) { return i.get<TImpl>(); });
                m_types.emplace(typeid(TInterface), std::move(e));
            }
        }

        /**
         * \brief Set handler for exceptions during service creation.
         * 
         * This handler function is called when an exception is encountered during service instantiation.
         * 
         * \param cb Exception handler
         */
        void set_except_handler(std::function<void(std::exception_ptr)> cb)
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            m_except_handler = cb;
        }

        /**
         * \brief Get the current exception handler
         * 
         * \return Exception handler
         */
        std::function<void(std::exception_ptr)> get_except_handler() const
        {
            std::unique_lock<std::recursive_mutex> lck(m_mtx);
            return m_except_handler;
        }
    };

    /**
     * \brief Set the default injector instance
     * \param i Default injector
     */
    void set_default_injector(std::shared_ptr<injector> i);

    /**
     * \brief Get the default injector instance
     * \return Current default injector or nullptr if none was assigned yet
     */
    std::shared_ptr<injector> get_default_injector();

    /**
     * \brief Get service of type T using the default injector.
     * 
     * If the service was not yet created it will try instanciate it using the builder defined at the bind statement.
     * If it can't create a valid instance for whatever reason it will return nullptr and the exception handler is called
     * with any error happening during creation.
     * \tparam T Type of the service interface
     * \return Returns instance of service or nullptr
     */
    template <typename T>
    inline static std::shared_ptr<T> get()
    {
        auto injector = get_default_injector();
        if (!injector)
            throw dependency_not_found_exception("No default injector set");
        return injector->get<T>();
    }

    /**
     * \brief Get service of type T using the default injector
     * 
     * Similar to the nothrow overload, but throws if it can't create a valid instance.
     * Always returns a non nullptr pointer.
     * \tparam T Type of the service interface
     * \return Returns instance of service
     */
    template <typename T>
    inline static std::shared_ptr<T> get(std::nothrow_t)
    {
        auto injector = get_default_injector();
        if (!injector)
            return nullptr;
        return injector->get<T>(std::nothrow);
    }

    /**
     * \brief Lazy service resolver
     * 
     * Resolves the given service on first access.
     */
    template <typename T>
    class lazy_handle
    {
        mutable std::shared_ptr<T> m_instance = nullptr;

    public:
        /**
         * Get an instance of this service and throw on error.
         * \return Instance of service
         */
        std::shared_ptr<T> get() const
        {
            auto x = std::atomic_load(&m_instance);
            if (!x)
            {
                x = tinydi::get<T>();
                std::atomic_store(&m_instance, x);
            }
            return x;
        }
        /**
         * Get an instance of this service and return nullptr on error.
         * \return Instance of service or nullptr
         */
        std::shared_ptr<T> get(std::nothrow_t) const
        {
            auto x = std::atomic_load(&m_instance);
            if (!x)
            {
                x = tinydi::get<T>(std::nothrow);
                if (x)
                    std::atomic_store(&m_instance, x);
            }
            return x;
        }
        /**
         * Reset the cached instance. Next access will cause a new service lookup.
         */
        void reset()
        {
            std::atomic_store(&m_instance, std::shared_ptr<T>{});
        }
        /**
         * Dereference the service. Looks up the given service implementation and returns it.
         * Throws on error.
         * \return Pointer to service.
         */
        T *operator->() const
        {
            return get().get();
        }
        /**
         * Dereference the service. Looks up the given service implementation and returns it.
         * Throws on error.
         * \return Reference to service.
         */
        T &operator*() const
        {
            return *get();
        }
    };
} // namespace tinydi

#define TINYDI_IMPL()                                                                      \
    namespace tinydi                                                                       \
    {                                                                                      \
        std::shared_ptr<injector> s_default_injector;                                      \
        void set_default_injector(std::shared_ptr<injector> i) { s_default_injector = i; } \
        std::shared_ptr<injector> get_default_injector() { return s_default_injector; }    \
    }
