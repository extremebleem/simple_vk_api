#include <curl/curl.h>
#include <iostream>
#include <codecvt>

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

	int replacedSpacesCount = 0;

	for (int i = 0; i < resultArguments.size() + replacedSpacesCount * 3; ++i)
	{
		wchar_t currentChar = resultArguments.at(i);
		if (currentChar == '&')
			break;

		if (currentChar == ' ')
		{
			std::string backup = resultArguments;
			resultArguments.erase(i);
			backup.erase(backup.begin(), backup.begin() + i + 1);
			resultArguments = resultArguments + "%20" + backup;
			++replacedSpacesCount;
		}
	}

	std::string response = MakeCurlRequest(resultURL, resultArguments);

	if (!response.empty())
	{
		return { nlohmann::json::parse(response) };
	}

	return { nullptr };
}

void VKApi::SetCaptchaCallback(captchaCallbackFn* callback)
{
	captchaCallback_ = callback;
}

int VKApi::CurlWriter(char* data, size_t size, size_t nmemb, std::string* buffer)
{
	buffer->append((char*)data, size * nmemb);
	return size * nmemb;
}

void dump(const char* text,
	FILE* stream, unsigned char* ptr, size_t size)
{
	size_t i;
	size_t c;
	unsigned int width = 0x10;

	fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
		text, (long)size, (long)size);

	for (i = 0; i < size; i += width) {
		fprintf(stream, "%4.4lx: ", (long)i);

		/* show hex to the left */
		for (c = 0; c < width; c++) {
			if (i + c < size)
				fprintf(stream, "%02x ", ptr[i + c]);
			else
				fputs("   ", stream);
		}

		/* show data on the right */
		for (c = 0; (c < width) && (i + c < size); c++) {
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			fputc(x, stream);
		}

		fputc('\n', stream); /* newline */
	}
}

int my_trace(CURL* handle, curl_infotype type, char* data, size_t size, void* userp)
{
	const char* text;
	(void)handle; /* prevent compiler warning */
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, stderr, (unsigned char*)data, size);
	return 0;
}

std::string VKApi::MakeCurlRequest(std::string url, std::string data)
{
	std::string curlDataBuffer = "";
	char curlErrorBuffer[CURL_ERROR_SIZE] = "";

	CURL* curl = curl_easy_init();

	if (!curl)
	{
		return "";
	}

	curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "accept: application/json");
	headers = curl_slist_append(headers, "content-type: application/x-www-form-urlencoded");

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36");
	curl_easy_setopt(curl, CURLOPT_DNS_SERVERS, "8.8.8.8:8.8.1.1");
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlErrorBuffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriter);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlDataBuffer);

	//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	CURLcode callResult = curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	if (callResult == CURLE_OK)
	{
		return curlDataBuffer;
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

std::string& MethodData::GetArgument(std::string name)
{
	return data_.find(name)->second;
}

const std::map<std::string, std::string>& MethodData::GetArguments() const
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