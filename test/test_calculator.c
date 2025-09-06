#include "test_macros.h"

#include "calculator.h"

#define ERROR_EPSILON (0.005)

static int IsMatch(double x, double y)
{
	return x >= y - ERROR_EPSILON && x <= y + ERROR_EPSILON;
}

static void TestCalculator(void)
{
	status_t status = SUCCESS;
	double result;

	status = Calculate("2 + 3", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 5.0), 1);
	status = Calculate("200-100+50.5", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 150.5), 1);
	status = Calculate("500", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 500), 1);
	status = Calculate("", &result);
	TEST("Status syntax error", status, INVALID_SYNTAX);
	status = Calculate("----", &result);
	TEST("Status syntax error", status, INVALID_SYNTAX);
	status = Calculate("        --				5", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 5), 1);
	status = Calculate("4--5", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 9), 1);
	status = Calculate("4--5 7 + 9", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("4--5+", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("4 * 5 / 4 + 20 - 5 * 4", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 5), 1);
	status = Calculate("4 * 5 // 4 + 20 - 5 * 4", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("4 ** 5 / 4 + 20 - 5 * 4", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("4 * 5 / 4 + 20 -- 5 * 4", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 45), 1);
	status = Calculate("0/0", &result);
	TEST("Status success", status, MATH_ERROR);
	status = Calculate("4 * 5 / (4 - 5))", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("4 * 5 / (4 - 5)", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, -20), 1);
	status = Calculate("4 * 5 / ((4 - 5)", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("4 * 5 / ((4 -5))", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, -20), 1);
	status = Calculate("(4 * 5 / 4 + 20 - 5 * 4)", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 5), 1);
	status = Calculate("(5 + 3) * 2", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 16), 1);
	status = Calculate("(5 + ) * 2", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("()", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("5(7)", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("(6 + 8)(6 + 4)", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("6+(+)7", &result);
	TEST("Status success", status, INVALID_SYNTAX);
	status = Calculate("(6 + 7) ^ 2", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 169), 1);
	status = Calculate("(2) ^ -2", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, 0.25), 1);
	status = Calculate("(0) ^ -1", &result);
	TEST("Status success", status, MATH_ERROR);
	status = Calculate("-5 ^ 2", &result);
	TEST("Status success", status, SUCCESS);
	TEST("Correct result", IsMatch(result, -25), 1);
}

int main(void)
{
	TestCalculator();
	PASS;
	return 0;
}
