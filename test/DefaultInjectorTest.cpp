#include <tinydi/tinydi.h>
#include <gtest/gtest.h>

namespace
{
    struct SampleInterface
    {
        virtual ~SampleInterface() {}
        virtual void hello() = 0;
    };

    struct SampleImpl : public SampleInterface
    {
        void hello() override
        {
            std::cout << "Hello" << std::endl;
        }
    };

    struct SampleImpl2 : public SampleInterface
    {
        void hello() override
        {
            std::cout << "Hello" << std::endl;
        }
    };
} // namespace

TEST(DefaultInjectorTest, SetGet)
{
    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    ASSERT_NE(tinydi::get_default_injector(), nullptr);
}

TEST(DefaultInjectorTest, GetService)
{
    tinydi::set_default_injector(nullptr);
    ASSERT_THROW(tinydi::get<SampleInterface>(), tinydi::dependency_not_found_exception);
    ASSERT_EQ(tinydi::get<SampleInterface>(std::nothrow), nullptr);

    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl>();
    ASSERT_NE(tinydi::get<SampleInterface>(), nullptr);
    ASSERT_NE(tinydi::get<SampleInterface>(std::nothrow), nullptr);
}

TEST(DefaultInjectorTest, GetAllServices)
{
    tinydi::set_default_injector(nullptr);
    ASSERT_THROW(tinydi::get_all<SampleInterface>(), tinydi::dependency_not_found_exception);
    ASSERT_EQ(tinydi::get_all<SampleInterface>(std::nothrow).size(), 0);

    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    ASSERT_EQ(tinydi::get_all<SampleInterface>().size(), 0);
    ASSERT_EQ(tinydi::get_all<SampleInterface>(std::nothrow).size(), 0);

    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl>();
    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl2>();
    ASSERT_EQ(tinydi::get_all<SampleInterface>().size(), 2);
    ASSERT_EQ(tinydi::get_all<SampleInterface>(std::nothrow).size(), 2);
}