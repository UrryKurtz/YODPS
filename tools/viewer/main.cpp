#include "YONode.h"
#include "YOViewer.h"
#include <iostream>
#include <string>
#include <map>


#include <Container/Ptr.h>
#include <Core/Context.h>

int main(int argc, char **argv)
{
    Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
    Urho3D::SharedPtr<YOViewer> application(new YOViewer(context));
    return application->Run();

    return 0;
}
