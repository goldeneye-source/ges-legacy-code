

#include "navGenerator.h"

int main()
{
	NavGenerator ng;

	ng.parseFile("simple_box.vmf");
	ng.saveFile("simple_box_map.obj");
	ng.genNavFile("simple_box_nav.obj");

	getchar();
}