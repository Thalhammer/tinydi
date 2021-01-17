#include <tinydi/tinydi.h>
#include <gtest/gtest.h>

namespace
{
    class SampleInterface
    {
        virtual void hello() = 0;
    };

    class SampleImpl : public SampleInterface
    {
        void hello() override
        {
            std::cout << "Hello" << std::endl;
        }
    };
} // namespace

TEST(InjectorTest, CreateInjector)
{
    tinydi::injector injector;
}

TEST(InjectorTest, BindFunction)
{
    tinydi::injector injector;
    injector.bind<SampleInterface>([](const tinydi::injector &) {
        return std::make_shared<SampleImpl>();
    });
}

TEST(InjectorTest, BindClass)
{
    tinydi::injector injector;
    injector.bind<SampleInterface, SampleImpl>();
}

TEST(InjectorTest, ReplaceFunction)
{
    tinydi::injector injector;
    injector.replace<SampleInterface>([](const tinydi::injector &) {
        return std::make_shared<SampleImpl>();
    });
}

TEST(InjectorTest, ReplaceClass)
{
    tinydi::injector injector;
    injector.replace<SampleInterface, SampleImpl>();
}

TEST(InjectorTest, ThrowsOnMissing)
{
    tinydi::injector injector;
    ASSERT_THROW(injector.get<SampleInterface>(), tinydi::dependency_not_found_exception);
}

TEST(InjectorTest, NoThrowGet)
{
    tinydi::injector injector;
    ASSERT_EQ(injector.get<SampleInterface>(std::nothrow), nullptr);
}

TEST(InjectorTest, Get)
{
    tinydi::injector injector;
    injector.bind<SampleInterface, SampleImpl>();
    ASSERT_NE(injector.get<SampleInterface>(), nullptr);
    ASSERT_NE(injector.get<SampleImpl>(), nullptr);
}

TEST(InjectorTest, GetNoThrow)
{
    tinydi::injector injector;
    injector.bind<SampleInterface, SampleImpl>();
    ASSERT_NE(injector.get<SampleInterface>(std::nothrow), nullptr);
    ASSERT_NE(injector.get<SampleImpl>(std::nothrow), nullptr);
}

TEST(InjectorTest, IsSame)
{
    tinydi::injector injector;
    auto inst = std::make_shared<SampleImpl>();
    injector.replace<SampleInterface>([inst](const tinydi::injector &) {
        return inst;
    });
    ASSERT_EQ(injector.get<SampleInterface>(), inst);
}

TEST(InjectorTest, Recursive)
{
    tinydi::injector injector;
    auto inst = std::make_shared<SampleImpl>();
    injector.replace<SampleInterface>([inst](const tinydi::injector &i) {
        i.get<SampleInterface>(std::nothrow);
        return inst;
    });
    ASSERT_EQ(injector.get<SampleInterface>(), inst);
}

TEST(InjectorTest, GetAll)
{
    tinydi::injector injector;
    injector.bind<SampleInterface>([](const tinydi::injector &) { return std::make_shared<SampleImpl>(); });
    injector.bind<SampleInterface>([](const tinydi::injector &) { return std::make_shared<SampleImpl>(); });
    auto all = injector.get_all<SampleInterface>();
    ASSERT_EQ(all.size(), 2);
    ASSERT_NE(all[0], nullptr);
    ASSERT_NE(all[1], nullptr);
    ASSERT_NE(all[0], all[1]);
}

TEST(InjectorTest, RecursiveGetAll)
{
    tinydi::injector injector;
    auto inst = std::make_shared<SampleImpl>();
    injector.replace<SampleInterface>([inst](const tinydi::injector &i) {
        i.get<SampleInterface>(std::nothrow);
        return inst;
    });
    auto all = injector.get_all<SampleInterface>();
    ASSERT_EQ(all.size(), 1);
    ASSERT_NE(all[0], nullptr);
}