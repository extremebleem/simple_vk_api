#include <iosfwd>

#include <nlohmann/json.hpp>

using captchaCallbackFn = std::string(*)(const std::string&);

struct ResponseData
{
	nlohmann::json data;
};

class MethodData
{
protected:
	std::string methodName_;
	std::map<std::string, std::string> data_;

public:
	void SetName(std::string name);
	std::string GetName() const;

	void AddArgument(std::string name, std::string value);
	std::string& GetArgument(std::string name);

	const std::map<std::string, std::string>& GetArguments() const;
	std::string GetArgumentsAsString() const;
};

class VKApi
{
	std::string accessToken_;
	std::string apiVersion_;
	std::string language_;

	std::string apiURL_;

	std::string requiredArguments_;

	captchaCallbackFn* captchaCallback_;

public:
	VKApi(std::string accessToken, std::string apiVersion, std::string language);
	
	ResponseData CallMethod(const MethodData& methodData);

	void SetCaptchaCallback(captchaCallbackFn* cap_callback);

private:
	static std::string WideStringToString(const std::string& wstr);
	static std::string StringToWideString(const std::string& wstr);
	std::string MakeCurlRequest(std::string url, std::string data);
	static int CurlWriter(char* data, size_t size, size_t nmemb, std::string* buffer);
};