# TinyDI
TinyDI is a tiny (one header, no source files) dependency injection framework.
Consider it a map of types with some extras.

## Binding a service
Binding an implementation to a interface is as easy as you'd expect it to be.
```c++
injector.bind<ServiceInterface, ServiceImplementation>();
```
This maps `ServiceInterface` to the implementation type `ServiceImplementation` and automatically creates an instance of it as soon as `ServiceInterface` is requested. It also registers `ServiceImplementation` with the injector, so you can quickly get the underlying implementation in case you really need to with out having to cast it.
You can also bind a function to act as an service provider.
```c++
injector.bind<ServiceInterface>([](const tinydi::injector& i){
    return std::make_shared<ServiceImplementation>();
});
```
This allows you to do more complex things and initialize classes which do not have a default constructor. The function will only be executed once and the result is cached inside injector for later lookups. If the interface is never requested the function is never called.

Both forms also exist as a form which replaces all existing implementations instead of adding an additional one called `replace` instead of `bind`.

## Getting a service
You can query an implementation of an interface using the `get` and `get_all` methods.
```c++
std::shared_ptr<ServiceInterface> service = injector.get<ServiceInterface>();
```
If there is an error instanciating the service or theres no implementation registered for the interface an exception is thrown.
```c++
std::shared_ptr<ServiceInterface> service = injector.get<ServiceInterface>(std::nothrow);
```
If you prefer to get a null pointer instead you can pass `std::nothrow` to the method. If there are multiple implementations registered to the interface the first one registered is returned. If you need to get all registered implementations you can call `get_all`. 
```c++
std::vector<std::shared_ptr<ServiceInterface>> service = injector.get_all<ServiceInterface>();
```

## Error handling
If instanciating the service fails and throws an exception tinydi will attempt to call it's error handler function if set and swallow the exception, behaving as if the service was never registered.
```c++
injector.set_except_handler([](std::exception_ptr e){
    std::terminate();
});
// Returns the handler just set
auto fn = injector.get_except_handler();
```

## Default injector
TinyDI has the concept of a "default" injector, which is really just a global instance of a regular injector used to provide some convenience features. If you want to use it you need to include the following macro in one of your source files.
```c++
TINYDI_IMPL()
```
Doing so allows you to call a global get method without having to pass around an injector and in places where you cant normally call methods with parameters (like function default arguments, member initializers). It also enables the use of `lazy_handle` which can be used a lazily resolve a service once you need it instead of on construction.

You can set the default injector using the following statement somewhere in your initialization code.
```c++
tinydi::set_default_injector(std::make_shared<tinydi::injector>());
// Returns the shared pointer just set
auto injector = tinydi::get_default_injector();
```
This allows all of the following forms of injection.
```c++
class dummy_class_construct {
    std::shared_ptr<ServiceInterface> m_service1;
    std::shared_ptr<ServiceInterface> m_service2 = tinydi::get<ServiceInterface>();
    tinydi::lazy_handle<ServiceInterface> m_service3;
public:
    dummy_class_construct(std::shared_ptr<ServiceInterface> s1 = tinydi::get<ServiceInterface>())
        : m_service1{s1}
    {}
};
```
Each of those forms has its own advantages and disadvantages, so you can mix and match as you like.