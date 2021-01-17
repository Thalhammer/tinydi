#include <tinydi/tinydi.h>
#include <iostream>

namespace di = tinydi;

struct NameService
{
    // Every registered service interface needs to be polymorphic.
    virtual ~NameService() {}
    std::string name {};
};

// This generates some boiler plate for the global instance.
// You need to include this in exactly 1 source file.
// If you dont use any of the convenience functions or the "default" injector you dont need this.
TINYDI_IMPL()

// This loads dependencies during construction, if no other implementation is passed as an
// argument to the constructor. You can use this if you want to make the use of DI optional
// without loosing most of the convenience. However this style might not play nice with some
// template magic which deducts the parameters.
class dummy_class_construct
{
public:
    std::shared_ptr<NameService> m_nameservice;
    dummy_class_construct(std::shared_ptr<NameService> nameservice = di::get<NameService>())
        : m_nameservice{nameservice}
    {
    }
};

// This also loads dependencies during construction, however you it will always use
// DI (normally). This is best used if you build your code around DI and dont want to
// or can not use the upper style.
class dummy_class_initialize
{
public:
    std::shared_ptr<NameService> m_nameservice = di::get<NameService>();
};

// This is different to the last two ways, as it does not resolve the dependency on
// construction. Instead it is resolved on first use. This is useful if you dont want
// require a service to be present or the service is not yet registered at construction
// (e.g. global objects). Note that dereferencing the handle will throw if it cant resolve
// the dependency.
class dummy_class_lazy
{
public:
    di::lazy_handle<NameService> m_nameservice {};
};

int main()
{
    // All of the convenience functions require a global "default" di injector,
    // so we set one.
    di::set_default_injector(std::make_shared<di::injector>());
    // Register our dummy service
    di::get_default_injector()->replace<NameService, NameService>();
    di::get<NameService>()->name = "Max";

    dummy_class_construct dummy_construct;
    // Should print Max
    std::cout << dummy_construct.m_nameservice->name << std::endl;

    dummy_class_initialize dummy_initialize;
    // Should print Max as well
    std::cout << dummy_initialize.m_nameservice->name << std::endl;

    dummy_class_lazy dummy_lazy;
    // Should print Max as well
    std::cout << dummy_lazy.m_nameservice->name << std::endl;
}