#pragma once
#include "./__ENV__.h"
#include "./ZERpc.h"
#include "./ZEConnection.h"

namespace ze
{

	class ZERpcProxy;
	using ZERpcPackBuffer = DynamicBuffer<ZERpc::HEADER_SIZE + 1024>;

	class ZERpcProxy
	: protected ZEConnectionEventListener
	{
	protected:
	struct ZERpcContext {
		friend class ZERpcProxy;
		private:
		size_t                xHeadReceived;
		size_t                xBodyExpected;
		size_t                xBodyReceived;
		ZERpc                 xRpcRequest;
		ZERpcPackBuffer       xRecvBuffer;
	};

	protected:
		ZE_API ZERpcProxy() = default;
		ZE_API ~ZERpcProxy() = default;

		ZE_API virtual ZE_FORCE_INLINE auto onConnected(ZEConnection * pConnection) -> ZERpcContext * { return new ZERpcContext{}; };
		ZE_API virtual ZE_FORCE_INLINE void onDisconnected(ZEConnection * pConnection, ZERpcContext * && pContext) { delete pContext; };

		ZE_API virtual bool postRequest(ZEConnection * pConnection, const ZERpc & rpc, const ubyte * dataBody);
		ZE_API virtual bool postRequest(ZEConnection * pConnection, const ZERpc & rpc);
		ZE_API virtual bool onRequest(ZEConnection * pConnection, const ZERpc & rpc, const ubyte * request) = 0;

		ZE_FORCE_INLINE ZERpcContext * getRpcContext(ZEConnection * pConnection) { return static_cast<ZERpcContext *>(pConnection->hConnectionEventListenerContext.ptr); }
		ZE_FORCE_INLINE void bindConnection(ZEConnection * pConnection, void * pContext = nullptr) {
			assert(!pConnection->hConnectionEventListener);
			pConnection->hConnectionEventListener = this;
			pConnection->hConnectionEventListenerContext.ptr = pContext;
		}
		ZE_FORCE_INLINE void unbindConnection(ZEConnection * pConnection) {
			assert(pConnection->hConnectionEventListener == this);
			pConnection->hConnectionEventListener = nullptr;
			pConnection->hConnectionEventListenerContext.ptr = nullptr;
		}

	private:
		bool onConnectionEstablished(ZEConnection * pConnection) override final {
			auto pContext = onConnected(pConnection);
			assert(pContext);
			pContext->xHeadReceived = 0;
			pContext->xBodyExpected = 0;
			pContext->xBodyReceived = 0;
			pContext->xRecvBuffer.reset();
			pConnection->hConnectionEventListenerContext.ptr = pContext;
			return true;
		}
		bool onConnectionInputReady(ZEConnection * pConnection) override final;
		void onConnectionActiveClose(ZEConnection * pConnection) override final {
			onDisconnected(pConnection, (ZERpcContext *)steal(pConnection->hConnectionEventListenerContext.ptr));
		}
		void onConnectionPassiveClose(ZEConnection * pConnection) override final {
			onDisconnected(pConnection, (ZERpcContext *)steal(pConnection->hConnectionEventListenerContext.ptr));
		}

		Result tryReadPackage(ZEConnection * pConnection);
		bool processData(const ZERpc & rpc, const ubyte * request);
	};

}
