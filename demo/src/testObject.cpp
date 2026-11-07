// Testing onMethodCall ... call as lambda instead of if ...

#include <string>
#include <vector>

#include "core/FunctionMap.h"
#include "core/VariableFrame.h"
#include "Globals.h"

using namespace DreiZehn;

const int TypeTestObject = RegisterUserObjectType("TestObject");

struct TestObject: public ValueObject {

  //NOTE unordered_map would be faster .. but for test lazy push_back and iterate also works.
  static inline std::vector<ValueObjectProperty> methods;

  Value mProperty;

  TestObject(): ValueObject(TypeTestObject) {}

  bool onMethodCall(uint32_t methodId, std::span<Value> args, Value& ret) override {
    for (auto& m: methods) {
      if (m.mIsMethod && m.mSymbolId == methodId) {
        return m.callMethod(this, methodId, args, ret) == 1;
      }
    }
    return ValueObject::onMethodCall(methodId,args,ret);
  }

  static void RegisterSymbols() {
    TestObject::methods.push_back(
      ValueObjectProperty("run", 1, 1, "Run a test", TypeTestObject,
                          [](ValueObject* object, std::span<Value> args, Value& ret) -> bool {
                            Tools::printf("TEST: ");
                            for (S32 i = 0; i < args[0].getInt(); i++) {
                              Tools::printf("%d ", i);
                            }
                            Tools::printf("done.\n");
                            ret = 4711;
                            return true;
                          })
    );
    TestObject::methods.push_back(
      ValueObjectProperty("time", 0, 0, "get the current time", TypeTestObject,
                          [](ValueObject* object, std::span<Value> args, Value& ret) -> bool {
                            ret = Value(std::string("no time here :P"));
                            return true;
                          })
    );

    TestObject::methods.push_back(
      ValueObjectProperty("prop", 0, 1, "get/set the property", TypeTestObject,
                          [](ValueObject* object, std::span<Value> args, Value& ret) -> bool {
                            // TestObject* sender = static_cast<TestObject*>(object);
                            TestObject* sender = (TestObject*)object;
                            if (args.size() > 0) sender->mProperty = args[0];
                            ret = sender->mProperty;
                            return true;
                          })
    );



  }

};
// --------------------------------------------------------------------------------------
void RegisterTest() {
  static bool registered = false; if (registered) return; registered = true;
  TestObject::RegisterSymbols();
   FunctionMap::RegisterFunction("Test::new", [](std::span<Value> args, Value& ret) -> bool {
      ret = Value(new TestObject());
      return true;
    });



}
