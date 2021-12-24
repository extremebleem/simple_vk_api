#include <curl/curl.h>
#include <iostream>

#include "api.h"

VKApi::VKApi(std::string accessToken, std::string apiVersion, std::string language)
	: accessToken_(accessToken), apiVersion_(apiVersion), language_(language), apiURL_("https://api.vk.com/method/"), captchaCallback_(nullptr)
{
	requiredArguments_ = "v=" + apiVersion_ + "&lang=" + language_ + "&access_token=" + accessToken_;
}

ResponseData VKApi::CallMethod(const MethodData& methodData)
{
	std::string resultURL = apiURL_ + methodData.GetName();
	std::string resultArguments = methodData.GetArgumentsAsString() + requiredArguments_;

	std::string response = MakeCurlRequest(resultURL.c_str(), resultArguments.c_str());

	return { nlohmann::json::parse(response) };
}

void VKApi::SetCaptchaCallback(captchaCallbackFn* callback)
{
	captchaCallback_ = callback;
}

int VKApi::CurlWriter(char* data, size_t size, size_t nmemb, std::string* buffer)
{
	int result = 0;

	if (buffer != NULL)
	{
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}

	return result;
}

const char* VKApi::MakeCurlRequest(const char* url, const char* data)
{
	std::string curlDataBuffer = "";
	char curlErrorBuffer[CURL_ERROR_SIZE] = "\0";

	CURL* curl = curl_easy_init();

	if (!curl)
	{
		return "";
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "VK API VKApi");
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriter);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlDataBuffer);

	CURLcode callResult = curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	if (callResult == CURLE_OK)
	{
		return std::move(curlDataBuffer.c_str());
	}

	return curlErrorBuffer;
}

void MethodData::SetName(std::string name)
{
	methodName_ = name;
}

std::string MethodData::GetName() const
{
	return methodName_;
}

void MethodData::AddArgument(std::string name, std::string value)
{
	data_.insert({ name, value });
}

const std::unordered_map<std::string, std::string>& MethodData::GetArguments() const
{
	return data_;
}

std::string MethodData::GetArgumentsAsString() const
{
	std::string returnString;

	for (const auto& [argName, argValue] : data_)
	{
		returnString += argName + "=" + argValue + "&";
	}

	return returnString;
}