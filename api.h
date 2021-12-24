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
	std::unordered_map<std::string, std::string> data_;

public:
	void SetName(std::string name);
	std::string GetName() const;

	void AddArgument(std::string name, std::string value);

	const std::unordered_map<std::string, std::string>& GetArguments() const;
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
	const char* MakeCurlRequest(const char* url, const char* data);
	static int CurlWriter(char* data, size_t size, size_t nmemb, std::string* buffer);

};