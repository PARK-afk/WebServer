#include "Error.hpp"

std::map<int, std::string> Error::errors;
std::map<int, std::string> Error::errorResponseBodyStrings;

void Error::initializeError() {

	errors[Continue_100] = "Continue";
	errors[SwitchingProtocol_101] = "Switching Protocol";
	errors[Processing_102] = "Processing";
	errors[EarlyHints_103] = "Early Hints";

	// Successful responses
	errors[OK_200] = "OK";
	errors[Created_201] = "Created";
	errors[Accepted_202] = "Accepted";
	errors[NonAuthoritativeInformation_203] = "Non-Authoritative Information";
	errors[NoContent_204] = "No Content";
	errors[ResetContent_205] = "Reset Content";
	errors[PartialContent_206] = "Partial Content";
	errors[MultiStatus_207] = "Multi-Status";
	errors[AlreadyReported_208] = "Already Reported";
	errors[IMUsed_226] = "IM Used";
	

	// Redirection messages
	errors[MultipleChoices_300] = "Multiple Choices";
	errors[MovedPermanently_301] = "Moved Permanently";
	errors[Found_302] = "Found";
	errors[SeeOther_303] = "See Other";
	errors[NotModified_304] = "Not Modified";
	errors[UseProxy_305] = "Use Proxy";
	errors[unused_306] = "unused";
	errors[TemporaryRedirect_307] = "Temporary Redirect";
	errors[PermanentRedirect_308] = "Permanent Redirect";

	// Client error responses
	errors[BadRequest_400] = "Bad Request";
	errors[Unauthorized_401] = "Unauthorized";
	errors[PaymentRequired_402] = "Payment Required";
	errors[Forbidden_403] = "Forbidden";
	errors[NotFound_404] = "Not Found";
	errors[MethodNotAllowed_405] = "Method Not Allowed";
	errors[NotAcceptable_406] = "Not Acceptable";
	errors[ProxyAuthenticationRequired_407] = "Proxy Authentication Required";
	errors[RequestTimeout_408] = "Request Timeout";
	errors[Conflict_409] = "Conflict";
	errors[Gone_410] = "Gone";
	errors[LengthRequired_411] = "Length Required";
	errors[PreconditionFailed_412] = "Precondition Failed";
	errors[PayloadTooLarge_413] = "Payload Too Large";
	errors[UriTooLong_414] = "URI Too Long";
	errors[UnsupportedMediaType_415] = "Unsupported Media Type";
	errors[RangeNotSatisfiable_416] = "Range Not Satisfiable";
	errors[ExpectationFailed_417] = "Expectation Failed";
	errors[ImATeapot_418] = "I'm a teapot";
	errors[MisdirectedRequest_421] = "Misdirected Request";
	errors[UnprocessableContent_422] = "Unprocessable Content";
	errors[Locked_423] = "Locked";
	errors[FailedDependency_424] = "Failed Dependency";
	errors[TooEarly_425] = "Too Early";
	errors[UpgradeRequired_426] = "Upgrade Required";
	errors[PreconditionRequired_428] = "Precondition Required";
	errors[TooManyRequests_429] = "Too Many Requests";
	errors[RequestHeaderFieldsTooLarge_431] = "Request Header Fields Too Large";
	errors[UnavailableForLegalReasons_451] = "Unavailable For Legal Reasons";

	// Server error responses
	errors[InternalServerError_500] = "Internal Server Error";
	errors[NotImplemented_501] = "Not Implemented";
	errors[BadGateway_502] = "Bad Gateway";
	errors[ServiceUnavailable_503] = "Service Unavailable";
	errors[GatewayTimeout_504] = "Gateway Timeout";
	errors[HttpVersionNotSupported_505] = "HTTP Version Not Supported";
	errors[VariantAlsoNegotiates_506] = "Variant Also Negotiates";
	errors[InsufficientStorage_507] = "Insufficient Storage";
	errors[LoopDetected_508] = "Loop Detected";
	errors[NotExtended_510] = "Not Extended";
	errors[NetworkAuthenticationRequired_511] = "Network Authentication Required";

	errorResponseBodyStrings[BadRequest_400] = "Your request is invalid.";
	errorResponseBodyStrings[Unauthorized_401] = "You are not authorized to access this page.";
	errorResponseBodyStrings[PaymentRequired_402] = "Payment is required to access this page.";
	errorResponseBodyStrings[Forbidden_403] = "You are forbidden from accessing this page.";
	errorResponseBodyStrings[NotFound_404] = "The page you are looking for is not found.";
	errorResponseBodyStrings[MethodNotAllowed_405] = "The method you are using is not allowed.";
	errorResponseBodyStrings[NotAcceptable_406] = "The page you are looking for is not acceptable.";
	errorResponseBodyStrings[ProxyAuthenticationRequired_407] = "Proxy authentication is required.";
	errorResponseBodyStrings[RequestTimeout_408] = "The request has timed out.";
	errorResponseBodyStrings[Conflict_409] = "There is a conflict in the request.";
	errorResponseBodyStrings[Gone_410] = "The page you are looking for is gone.";
	errorResponseBodyStrings[LengthRequired_411] = "The length of the request is required.";
	errorResponseBodyStrings[PreconditionFailed_412] = "The precondition of the request has failed.";
	errorResponseBodyStrings[PayloadTooLarge_413] = "The payload of the request is too large.";
	errorResponseBodyStrings[UriTooLong_414] = "The URI of the request is too long.";
	errorResponseBodyStrings[UnsupportedMediaType_415] = "The media type of the request is unsupported.";
	errorResponseBodyStrings[RangeNotSatisfiable_416] = "The range of the request is not satisfiable.";
	errorResponseBodyStrings[ExpectationFailed_417] = "The expectation of the request has failed.";
	errorResponseBodyStrings[Locked_423] = "The request is locked.";
	errorResponseBodyStrings[ImATeapot_418] = "I'm a teapot.";
	errorResponseBodyStrings[MisdirectedRequest_421] = "The request is misdirected.";
	errorResponseBodyStrings[UnprocessableContent_422] = "The content of the request is unprocessable.";
	errorResponseBodyStrings[FailedDependency_424] = "The request has failed due to dependency.";
	errorResponseBodyStrings[TooEarly_425] = "The request is too early.";
	errorResponseBodyStrings[UpgradeRequired_426] = "The request requires an upgrade.";
	errorResponseBodyStrings[PreconditionRequired_428] = "The precondition of the request is required.";
	errorResponseBodyStrings[TooManyRequests_429] = "The request has too many requests.";
	errorResponseBodyStrings[RequestHeaderFieldsTooLarge_431] = "The header fields of the request are too large.";
	errorResponseBodyStrings[UnavailableForLegalReasons_451] = "The request is unavailable for legal reasons.";

	//500
	errorResponseBodyStrings[InternalServerError_500] = "Internal server error.";
	errorResponseBodyStrings[NotImplemented_501] = "The request is not implemented.";
	errorResponseBodyStrings[BadGateway_502] = "The gateway is bad.";
	errorResponseBodyStrings[ServiceUnavailable_503] = "The service is unavailable.";
	errorResponseBodyStrings[GatewayTimeout_504] = "The gateway has timed out.";
	errorResponseBodyStrings[HttpVersionNotSupported_505] = "The HTTP version is not supported.";
	errorResponseBodyStrings[VariantAlsoNegotiates_506] = "The variant also negotiates.";
	errorResponseBodyStrings[InsufficientStorage_507] = "The storage is insufficient.";
	errorResponseBodyStrings[LoopDetected_508] = "The loop is detected.";
	errorResponseBodyStrings[NotExtended_510] = "The request is not extended.";
	errorResponseBodyStrings[NetworkAuthenticationRequired_511] = "The network authentication is required.";
}

Error::Error() {}

std::string Error::createErrorResponse(int code) {
	Response response;
	response.setStatusCode(code);
	response.setHeader("Content-Type", "text/html; charset=utf-8");
	response.setHeader("Date", Manager::responseUtils.getCurTime());
	std::string errorBody = "<html><body><h1>" + Manager::utils.toString(code) + " " + errors[code] + "</h1><p>" + Error::errorResponseBodyStrings[code] + "</p></body></html>";
	response.setBody(errorBody);
	response.setHeader("Content-Length", Manager::utils.toString(errorBody.length()));
	response.setHeader("Connection", "close");
	return response.getResponses();
}

const std::string& Error::operator[](int status) const {
	return errors.at(status);
}

std::string Error::getError(int status) {
	return errors[status];
}

std::string Error::errorHandler(const ServerConfig &Serv, int num) {
	std::string fileName;
	std::string testPath;
	Response response;

	fileName = "/" + Manager::utils.toString(num) + ".html";
	testPath = Serv.getErrorPagesPath() + fileName; 
	std::ifstream file(testPath.c_str());
	
	if (file.is_open() && file.good()) {
		std::streamsize fileSize = Manager::responseUtils.getFileSize(file);

		std::string body = Manager::responseUtils.readFileContent(file, fileSize);
		file.close();

		response.setStatusCode(NotFound_404);
		response.setHeader("Date", Manager::responseUtils.getCurTime());
		response.setHeader("Content-Type", Manager::responseUtils.getContentType("html"));
		response.setBody(body);
		response.setHeader("Content-Length", Manager::utils.toString(body.length()));
		response.setHeader("Connection", "close");
		return response.getResponses();
	}
	else 
		return createErrorResponse(num);
}
