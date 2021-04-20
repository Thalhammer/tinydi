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
    struct SampleInterface2
    {
        virtual ~SampleInterface2() {}
        virtual void hello() = 0;
    };

    struct SampleImpl2 : public SampleInterface2
    {
        void hello() override
        {
            std::cout << "Hello" << std::endl;
        }
    };
} // namespace

TINYDI_BIND_CLASS(SampleInterface, SampleImpl)
TINYDI_BIND_FUNCTION(SampleInterface2, [](){
    return std::make_shared<SampleImpl2>();
})
TINYDI_BIND_FUNCTION(SampleInterface2, [](const ::tinydi::injector&){
    return std::make_shared<SampleImpl2>();
})

TEST(StaticMappingTest, BindStaticMappings)
{
    tinydi::injector di;
    ASSERT_EQ(di.get<SampleInterface>(std::nothrow), nullptr);
    auto get_all = di.get_all<SampleInterface2>();
    ASSERT_EQ(get_all.size(), 0);
    di.bind_static_mappings();
    ASSERT_NE(di.get<SampleInterface>(), nullptr);
    ASSERT_NE(di.get<SampleImpl>(), nullptr);
    ASSERT_EQ(di.get<SampleInterface>(), di.get<SampleImpl>());
    get_all = di.get_all<SampleInterface2>();
    ASSERT_EQ(get_all.size(), 2);
    ASSERT_NE(get_all[0], nullptr);
    ASSERT_NE(get_all[1], nullptr);
}
