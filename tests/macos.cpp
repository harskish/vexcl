#define BOOST_TEST_MODULE CustomKernel
#include <boost/test/unit_test.hpp>
#include <vexcl/vector.hpp>
#include <vexcl/function.hpp>
#include "context_setup.hpp"

BOOST_AUTO_TEST_CASE(custom_kernel)
{
    const cl_ulong n = 1024;

    std::vector<vex::command_queue> queue(1, ctx.queue(0));

    vex::vector<int> x(queue, n);

    // Single kernel per program
    {
        vex::backend::source_generator src(queue[0]);

        src << R"(
        int4 make_int4
        (
          int x
        )
        {
          return (int4)(x, x, x, x);
        })";
        src.begin_kernel("the_answer");
        src.begin_kernel_parameters();
        src.parameter<size_t>("n");
        src.parameter<int*>("x");
        src.end_kernel_parameters();
        src.new_line() << "int4 x4 = make_int4(42);";
        src.new_line() << "for (ulong idx = get_global_id(0); idx < n; idx += get_global_size(0))";
        src.open("{");
        src.new_line() << "x[idx] = x4.x;";
        src.close("}");
        src.end_kernel();

        vex::backend::kernel zeros(queue[0], src.str(), "the_answer");

#ifdef BOOST_NO_VARIADIC_TEMPLATES
        zeros.push_arg(n);
        zeros.push_arg(x(0));
        zeros(queue[0]);
#else
        zeros(queue[0], n, x(0));
#endif

        check_sample(x, [](size_t, int v) { BOOST_CHECK_EQUAL(v, 42); });
    }
}

BOOST_AUTO_TEST_SUITE_END()
