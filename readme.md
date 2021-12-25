# **SIMPLE VK API**
Простенькая библиотека для использования [VK API](https://dev.vk.com/method)
## Зависимости
+ nlohmann JSON
+ CURL compiled with Windows SSPI & WinIDN
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