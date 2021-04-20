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
} // namespace

TEST(DefaultInjectorTest, SetGet)
{
    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    ASSERT_NE(tinydi::get_default_injector(), nullptr);
}

TEST(DefaultInjectorTest, GetService)
{
    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl>();
    ASSERT_NE(tinydi::get<SampleInterface>(), nullptr);
    ASSERT_NE(tinydi::get<SampleInterface>(std::nothrow), nullptr);
}