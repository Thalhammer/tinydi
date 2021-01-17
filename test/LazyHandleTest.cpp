#include <tinydi/tinydi.h>
#include <gtest/gtest.h>

namespace
{
    struct SampleInterface
    {
        virtual void hello() = 0;
    };

    struct SampleImpl : public SampleInterface
    {
        void hello() override
        {
            ASSERT_NE(this, nullptr);
        }
    };
} // namespace

TEST(LazyHandleTest, ArrowOperator)
{
    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl>();

    tinydi::lazy_handle<SampleInterface> m_interface;
    m_interface->hello();
}

TEST(LazyHandleTest, Deref)
{
    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl>();

    tinydi::lazy_handle<SampleInterface> m_interface;
    (*m_interface).hello();
}

TEST(LazyHandleTest, Get)
{
    tinydi::set_default_injector(std::make_shared<tinydi::injector>());
    tinydi::get_default_injector()->bind<SampleInterface, SampleImpl>();

    tinydi::lazy_handle<SampleInterface> m_interface;
    ASSERT_NE(m_interface.get(), nullptr);
}
