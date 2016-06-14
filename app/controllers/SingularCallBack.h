template <class Class, typename ReturnType, typename Parameter, typename Parameter2>
class SingularCallBack
{
public:

   typedef ReturnType (Class::*Method)(Parameter, Parameter2);

   SingularCallBack(Class* class_instance, Method method)
   : class_instance_(class_instance),
     method_(method)
   {}

   SingularCallBack()
   {}

   ReturnType operator()(Parameter parameter, Parameter2 parameter2)
   {
      return (class_instance_->*method_)(parameter, parameter2);
   }

   ReturnType execute(Parameter parameter, Parameter2 parameter2)
   {
      return operator()(parameter, parameter2);
   }

private:

   Class* class_instance_;
   Method method_;
};
