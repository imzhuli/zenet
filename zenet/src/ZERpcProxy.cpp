#include <zenet/ZERpcProxy.h>
#include <ze/convention.h>
#include <ze/raw.h>
#include <ze/ext/string.h>

namespace ze
{

	bool ZERpcProxy::onConnectionInputReady(ZEConnection * pConnection)
	{
		Result result;
		do {
			result = tryReadPackage(pConnection);
			if (result != Result::AGAIN) {
				break;
			}
		} while(true);
		return result >= Result::OK;
	}

	Result ZERpcProxy::tryReadPackage(ZEConnection * pConnection)
	{
		auto & rRpcContext   = *getRpcContext(pConnection);
		auto & rHeadReceived = rRpcContext.xHeadReceived;
		auto & rBodyExpected = rRpcContext.xBodyExpected;
		auto & rBodyReceived = rRpcContext.xBodyReceived;
		auto & rZERpcRequest = rRpcContext.xRpcRequest;
		auto & rRecvBuffer   = rRpcContext.xRecvBuffer;

		if (size_t headerExpected = ZERpc::HEADER_SIZE - rHeadReceived) { // receive header
			pConnection->read(rRecvBuffer + rHeadReceived, headerExpected);
			rHeadReceived += headerExpected;
			headerExpected = ZERpc::HEADER_SIZE - rHeadReceived;
			if (headerExpected) { // header not done
				return Result::OK;
			}
		}

		// check header and extract body length:
		if (!rBodyExpected) {
			if (!rZERpcRequest.readHeader(rRecvBuffer)
			|| !rRecvBuffer.resize(rZERpcRequest.totalSize())) {
				return Result::GENERIC_ERROR;
			}
			rBodyExpected = rZERpcRequest.bodySize();
		}

		auto pBody = rRecvBuffer + ZERpc::HEADER_SIZE;
		if (size_t bodyExpected = rBodyExpected - rBodyReceived) {
			pConnection->read(pBody + rBodyReceived, bodyExpected);
			rBodyReceived += bodyExpected;
			bodyExpected = rBodyExpected - rBodyReceived;
			if (bodyExpected) {
				return Result::OK;
			}
		}

		// body received:
		if (!onRequest(pConnection, rZERpcRequest, rBodyReceived ? pBody : nullptr)) {
			return Result::GENERIC_ERROR;
		}

		// cleanup:
		rRecvBuffer.reset();
		rHeadReceived = 0;
		rBodyExpected = 0;
		rBodyReceived = 0;
		return Result::AGAIN;
	}

	ZE_API bool ZERpcProxy::postRequest(ZEConnection * pConnection, const ZERpc & rpc, const ubyte * dataBody)
	{
		ubyte xHeader[ZERpc::HEADER_SIZE];
		rpc.writeHeader(xHeader);
		DataView writeList[] = {
			{xHeader}, { dataBody, rpc.bodySize() }
		};
		return pConnection->write(writeList);
	}

	ZE_API bool ZERpcProxy::postRequest(ZEConnection * pConnection, const ZERpc & rpc)
	{
		assert(!rpc.bodySize());
		ubyte xHeader[ZERpc::HEADER_SIZE];
		rpc.writeHeader(xHeader);
		return pConnection->write(xHeader, sizeof(xHeader));
	}

}
