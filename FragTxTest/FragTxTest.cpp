#include "pch.h"
#include "CppUnitTest.h"
#include "../bit_array.h"
#include <stdio.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace FragTxTest
{
	TEST_CLASS(FragTxTest)
	{
	public:
		
		TEST_METHOD(test_get_bit_array)
		{
			bit_array_t b1;
			bit_array_t* b1p = &b1;

			bool res = get_bit_array(b1p, 12);

			printf("%u", b1.numBits);

	

		}
	};
}
