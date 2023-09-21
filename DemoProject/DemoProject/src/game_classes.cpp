#include "game_api.h"
#include "classes/class.h"

/*
	Include your classes here...

	#include "myclass1.h"
	#include "myclass2.h"
	#include "myclass3.h"


*/

#include "demo_rotating_component.h"


GAME_CLASS_LIST_BEGIN{
GAME_API CLASS_LIST_BEGIN(GAME_CLASSES_LIST)
{
	/*
		Add your classes here...


		CLASS_LIST_ADD(MyClass1),
		CLASS_LIST_ADD(MyClass2),
		CLASS_LIST_ADD(MyClass3)


	*/

	CLASS_LIST_ADD(DemoRotatingComponent)

}
CLASS_LIST_END() }