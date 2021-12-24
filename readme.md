# **SIMPLE VK API**
## Зависимости
+ nlohmann JSON
+ CURL
## Пример использования
	int main()
	{
		VKApi api = VKApi("YOUR ACCESS TOKEN", "5.131", "ru");

		MethodData setStatusMethod;

		setStatusMethod.SetName("status.set");
		setStatusMethod.AddArgument("text", "keklol");
		
		ResponseData responseData = api.CallMethod(setStatusMethod);

		std::cout << responseData.data.at("response").get<int>();

		return 0;
	}