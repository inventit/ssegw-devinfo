/*
 * LEGAL NOTICE
 *
 * Copyright (C) 2012-2015 InventIt Inc. All rights reserved.
 *
 * This source code, product and/or document is protected under licenses
 * restricting its use, copying, distribution, and decompilation.
 * No part of this source code, product or document may be reproduced in
 * any form by any means without prior written authorization of InventIt Inc.
 * and its licensors, if any.
 *
 * InventIt Inc.
 * 9F KOJIMACHI CP BUILDING
 * 4-4-7 Kojimachi, Chiyoda-ku, Tokyo 102-0083
 * JAPAN
 * http://www.yourinventit.com/
 */

#ifndef __DEVINFO_MODEL_COMMAND_H__
#define __DEVINFO_MODEL_COMMAND_H__

SSE_BEGIN_C_DECLS

struct TDEVINFOModelCommand_ {
  Moat fMoat;
  TDEVINFOManager *fMgr;
  sse_char *fUid;
  sse_char *fKey;
  sse_char *fCommand;
  MoatValue *fParam;
};
typedef struct TDEVINFOModelCommand_ TDEVINFOModelCommand;

TDEVINFOModelCommand*
DEVINFOModelCommand_New(Moat in_moat,
			TDEVINFOManager *in_mgr,
			sse_char *in_uid,
			sse_char *in_key,
			sse_char *in_command,
			MoatValue *in_param);

void
TDEVINFOModelCommand_Delete(TDEVINFOModelCommand *self);

SSE_END_C_DECLS

#endif /* __DEVINFO_MODEL_COMMAND_H__ */

