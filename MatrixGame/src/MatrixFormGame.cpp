// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

#include "stdafx.h"
#include "MatrixFormGame.hpp"
#include "MatrixMap.hpp"
#include "MatrixRobot.hpp"
#include "MatrixFlyer.hpp"
#include "Effects/MatrixEffect.hpp"
#include "Interface/CInterface.h"
#include "MatrixMultiSelection.hpp"
#include "MatrixMapStatic.hpp"
#include "Interface/CIFaceMenu.h"
#include "MatrixGameDll.hpp"
#include "MatrixInstantDraw.hpp"
#include "Interface/MatrixHint.hpp"
#include "MatrixObjectCannon.hpp"
#include "Interface/CCounter.h"
#include "Interface/CIFaceButton.h"
#include "Interface/CHistory.h"

#include <time.h>
#include <sys/timeb.h>
#include "stdio.h"

CFormMatrixGame::CFormMatrixGame() :CForm()
{
    DTRACE();
    m_Name = L"MatrixGame";

    m_LastWorldX = 0; m_LastWorldY = 0;
    m_Action = 0;
    ZeroMemory(m_LastScans, sizeof(SKeyScan) * MAX_SCANS);
}

CFormMatrixGame::~CFormMatrixGame()
{
    DTRACE();
}

void CFormMatrixGame::Enter(void)
{
    DTRACE();
    S3D_Default();
    D3DMATERIAL9 mtrl;
    ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    mtrl.Specular.r = 0.5f;
    mtrl.Specular.g = 0.5f;
    mtrl.Specular.b = 0.5f;
    mtrl.Specular.a = 0.5f;
    g_D3DD->SetMaterial(&mtrl);
    g_D3DD->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);

    D3DXVECTOR3 vecDir;
    D3DLIGHT9 light;
    ZeroMemory(&light, sizeof(D3DLIGHT9));
    light.Type = D3DLIGHT_DIRECTIONAL;//D3DLIGHT_POINT;//D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Diffuse.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    light.Ambient.r = 0.0f;
    light.Ambient.g = 0.0f;
    light.Ambient.b = 0.0f;
    light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
    light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
    light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
    //light.Range       = 0;
    light.Direction = g_MatrixMap->m_LightMain;
    //	light.Direction=D3DXVECTOR3(250.0f,-50.0f,-250.0f);
    //	D3DXVec3Normalize((D3DXVECTOR3 *)(&(light.Direction)),(D3DXVECTOR3 *)(&(light.Direction)));
    ASSERT_DX(g_D3DD->SetLight(0, &light));
    ASSERT_DX(g_D3DD->LightEnable(0, TRUE));

    g_MatrixMap->m_Cursor.SetVisible(true);
}

void CFormMatrixGame::Leave(void)
{
    DTRACE();
}

void CFormMatrixGame::Draw(void)
{
    DTRACE();

    if(!FLAG(g_MatrixMap->m_Flags, MMFLAG_VIDEO_RESOURCES_READY)) return;

    CInstDraw::DrawFrameBegin();

    if(FLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE))
    {
        g_MatrixMap->m_DI.T(L"Automatic mode", L"");
    }

    if(FLAG(g_Config.m_DIFlags, DI_DRAWFPS)) g_MatrixMap->m_DI.T(L"FPS", CWStr(g_DrawFPS, g_MatrixHeap).Get());
    if(FLAG(g_Config.m_DIFlags, DI_TMEM))
    {
        g_MatrixMap->m_DI.T(L"Free Texture Mem", CWStr(g_AvailableTexMem, g_MatrixHeap).Get());
    }
    if(FLAG(g_Config.m_DIFlags, DI_TARGETCOORD))
    {
        CWStr txt(g_MatrixHeap);
        txt = Float2Int(g_MatrixMap->m_Camera.GetXYStrategy().x * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        txt += L", ";
        txt += Float2Int(g_MatrixMap->m_Camera.GetXYStrategy().y * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        //txt += L", ";
        //txt += Float2Int((g_MatrixMap->m_Camera.GetTarget().z+g_MatrixMap->m_Camera.GetZRel()) * 10.0f);
        //txt.Insert(txt.GetLen()-1,L".",1);
        g_MatrixMap->m_DI.T(L"Camera target", txt.Get());
    }
    if(FLAG(g_Config.m_DIFlags, DI_FRUSTUMCENTER))
    {
        CWStr txt(g_MatrixHeap);

        txt = Float2Int(g_MatrixMap->m_Camera.GetFrustumCenter().x * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        txt += L", ";
        txt += Float2Int(g_MatrixMap->m_Camera.GetFrustumCenter().y * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        txt += L", ";
        txt += Float2Int(g_MatrixMap->m_Camera.GetFrustumCenter().z * 10.0f);
        txt.Insert(txt.GetLen() - 1, L".", 1);
        g_MatrixMap->m_DI.T(L"Frustum Center", txt.Get());


        //txt = Float2Int(D3DXVec3Length(&(g_MatrixMap->m_Camera.GetFrustumCenter()-(g_MatrixMap->m_Camera.GetTarget()+D3DXVECTOR3(0,0,g_MatrixMap->m_Camera.GetZRel())))) * 10.0f);
        //txt.Insert(txt.GetLen()-1,L".",1);
        //g_MatrixMap->m_DI.T(L"Cam dist",txt.Get());

        //g_MatrixMap->m_DI.T(L"Z rel",CWStr(g_MatrixMap->m_Camera.GetZRel()));

    }

	g_MatrixMap->BeforeDraw();


	//ASSERT_DX(g_D3DD->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, D3DCOLOR_XRGB(255,0,0), 1.0f, 0 ));
    
    if (FLAG(g_Flags, GFLAG_STENCILAVAILABLE))
    {
#if defined _DEBUG || defined EXE_VERSION
        ASSERT_DX(g_D3DD->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL | D3DCLEAR_TARGET, D3DCOLOR_XRGB(255,0,255), 1.0f, 0 ));
#else
        ASSERT_DX(g_D3DD->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0 ));
#endif

        if (CInterface::ClearRects_GetCount()) ASSERT_DX(g_D3DD->Clear( CInterface::ClearRects_GetCount(), CInterface::ClearRects_Get(), D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL , D3DCOLOR_XRGB(255,0,255), 0.0f, 0 ));
    }
    else
    {
        ASSERT_DX(g_D3DD->Clear( 0, NULL, D3DCLEAR_ZBUFFER , D3DCOLOR_XRGB(255,0,255), 1.0f, 0 ));

        if (CInterface::ClearRects_GetCount()) ASSERT_DX(g_D3DD->Clear( CInterface::ClearRects_GetCount(), CInterface::ClearRects_Get(), D3DCLEAR_ZBUFFER , D3DCOLOR_XRGB(255,0,255), 0.0f, 0 ));
    }

	ASSERT_DX(g_D3DD->BeginScene());
#ifdef _DEBUG
    if (!FLAG(g_Flags, GFLAG_EXTRAFREERES))
    {
        SETFLAG(g_Flags, GFLAG_RENDERINPROGRESS);
    }
#endif


	g_MatrixMap->Draw();
	

	ASSERT_DX(g_D3DD->EndScene());

    //SETFLAG(g_Flags, GFLAG_PRESENT_REQUIRED);
    static int current_delay = 0;

    static DWORD last_time = 0;
    DWORD ctime = timeGetTime();
    int step1 = (last_time <= ctime) ? (ctime - last_time) : (0xFFFFFFFF - last_time + ctime);
    last_time = ctime;

    float cfps = 1000.0f / float(step1);

    if (cfps > float(g_MaxFPS))
    {
        ++current_delay;
    }
    else
    {
        --current_delay;
        if (current_delay < 0) current_delay = 0;
    }

    Sleep(current_delay);
    //if (FLAG(g_Flags, GFLAG_PRESENT_REQUIRED))
    //{
    //    ASSERT_DX(g_D3DD->Present(NULL,NULL,NULL,NULL));
    //    RESETFLAG(g_Flags, GFLAG_PRESENT_REQUIRED);
    //}
    ASSERT_DX(g_D3DD->Present(NULL,NULL,NULL,NULL));


#ifdef _DEBUG
    RESETFLAG(g_Flags, GFLAG_RENDERINPROGRESS);
    RESETFLAG(g_Flags, GFLAG_EXTRAFREERES);
#endif



#ifdef MEM_SPY_ENABLE
    CDText::T("ALLOC", CStr((int)SMemHeader::fullsize));

    char buff[1024];
    CDText::Get(buff);

    SetWindowTextA(g_Wnd, buff);
#endif
    
}

void CFormMatrixGame::Takt(int step)
{
    DTRACE();

    if(g_MatrixMap->CheckLostDevice()) return;

	g_MatrixMap->Takt(step);

    CPoint mp = g_MatrixMap->m_Cursor.GetPos();

    if(!g_MatrixMap->GetPlayerSide()->IsArcadeMode())
    {
        if(mp.x >= 0 && mp.x < g_ScreenX && mp.y >= 0 && mp.y < g_ScreenY)
        {
            if(mp.x < MOUSE_BORDER) g_MatrixMap->m_Camera.MoveLeft();
            if(mp.x > (g_ScreenX - MOUSE_BORDER)) g_MatrixMap->m_Camera.MoveRight();
            if(mp.y < MOUSE_BORDER) g_MatrixMap->m_Camera.MoveUp();
            if(mp.y > (g_ScreenY - MOUSE_BORDER)) g_MatrixMap->m_Camera.MoveDown();
        }
    }

    if(g_MatrixMap->m_Console.IsActive()) return;

    if(!g_MatrixMap->GetPlayerSide()->IsArcadeMode())
    {
        //Перед движением камеры по клавишам делаем проверку на то, что игрок не находится в режиме конструктора
        //В режиме конструктора стандартные кнопки движения камеры используются под другие бинды
        if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_LEFT]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_LEFT_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveLeft();
        }
        if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_RIGHT]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_RIGHT_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveRight();
        }
        if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_UP]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_UP_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveUp();
        }
        if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_DOWN]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_SCROLL_DOWN_ALT]) & 0x8000) == 0x8000))
        {
            if(!g_MatrixMap->GetPlayerSide()->m_ConstructPanel->IsActive()) g_MatrixMap->m_Camera.MoveDown();
        }
        if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_LEFT]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_LEFT_ALT]) & 0x8000) == 0x8000))
        {
            g_MatrixMap->m_Camera.RotLeft();
        }
        if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_RIGHT]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_RIGHT_ALT]) & 0x8000) == 0x8000))
        {
            g_MatrixMap->m_Camera.RotRight();
        }
    }

    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_UP]) & 0x8000) == 0x8000)
    {
        g_MatrixMap->m_Camera.RotUp();
    }
    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ROTATE_DOWN]) & 0x8000) == 0x8000)
    {
        g_MatrixMap->m_Camera.RotDown();
    }

    //Создаётся и сохраняется игровой скриншот (не заносится в буфер обмена)
    if(GetAsyncKeyState(g_Config.m_KeyActions[KA_SAVE_SCREENSHOT]))
    {
        CreateDirectory(PathToOutputFiles(FOLDER_NAME_SCREENSHOTS), NULL);

        // seek files

        int maxn = 0;

        WIN32_FIND_DATA fd;

        int slen = strlen(FILE_NAME_SCREENSHOT);
		
        CStr n(PathToOutputFiles(FOLDER_NAME_SCREENSHOTS), g_CacheHeap);
		
        n += "\\";
        n.Add(FILE_NAME_SCREENSHOT, slen);
        n += "*.*";

        HANDLE ff = FindFirstFile(n.Get(), &fd);
        if(ff != INVALID_HANDLE_VALUE)
        {
            while(true)
            {
                if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    n = fd.cFileName;
                    int idx = n.FindR(FILE_NAME_SCREENSHOT, slen);
                    n.Del(0, idx + slen);
                    idx = n.Find(".", 1);
                    if(idx > 0) n.SetLen(idx);
                    int nn = n.GetInt();
                    if(nn > maxn) maxn = nn;
                }
                if(0 == FindNextFile(ff, &fd)) break;
            }
            FindClose(ff);
            ++maxn;
        }

        n.Set( maxn );
        n.Insert(0,"000",3);
        n.Del(0,n.Len() - 3);
		/*
		CStr    n("",g_CacheHeap);

		struct _timeb timebuffer;
        time_t time1;
        unsigned short millitm1;

        _ftime64( &timebuffer );

        time1 = timebuffer.time;
        millitm1 = timebuffer.millitm;
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&time1);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H.%M.%S.", &tstruct);
		n.Set( millitm1 );
        n.Insert(0,"000",3);
        n.Del(0,n.Len() - 3);

		n.Insert(0,buf,20);
		n.Insert(0," ",1);*/

        n.Insert(0, FILE_NAME_SCREENSHOT, slen);
        n.Insert(0, "\\", 1);
        n.Insert(0, PathToOutputFiles(FOLDER_NAME_SCREENSHOTS));
        n.Add(".png", 4);

        DeleteFile(n.Get());

        if(!g_D3Dpp.Windowed)
        {
            IDirect3DSurface9* pTargetSurface = NULL;
            HRESULT hr = D3D_OK;

            if(!g_D3Dpp.MultiSampleType)
            hr = g_D3DD->GetRenderTarget(0, &pTargetSurface);

            if(hr == D3D_OK)
            {
                D3DSURFACE_DESC desc;

                if(!g_D3Dpp.MultiSampleType)
                {
                    hr = pTargetSurface->GetDesc(&desc);
                }
                else
                {
                    desc.Width = g_ScreenX;
                    desc.Height = g_ScreenY;
                    desc.Format = D3DFMT_A8R8G8B8;
                }
                if(hr == D3D_OK)
                {
                    IDirect3DSurface9 * pSurface = NULL;
                    hr = g_D3DD->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL);
                    if(hr == D3D_OK)
                    {
                        if(!g_D3Dpp.MultiSampleType)
                        {
                            hr = g_D3DD->GetRenderTargetData(pTargetSurface, pSurface);
                        }
                        else
                        {
                            hr = g_D3DD->GetFrontBufferData(0, pSurface);
                        }
                        if(hr == D3D_OK)
                        {
                            D3DLOCKED_RECT lockrect;
                            hr = pSurface->LockRect(&lockrect, NULL, 0);
                            if(hr == D3D_OK)
                            {
                                CBitmap bm;
                                bm.CreateRGB(desc.Width, desc.Height);

                                for(UINT y = 0; y < desc.Height; ++y)
                                {
                                    unsigned char* buf_src = (unsigned char*)lockrect.pBits + lockrect.Pitch * y;
                                    unsigned char* buf_des = (unsigned char*)bm.Data() + bm.Pitch() * y;

                                    for(UINT x = 0; x < desc.Width; ++x)
                                    {
                                        //memcpy(buf_des, buf_src, 3);
                                        buf_des[0] = buf_src[2];
                                        buf_des[1] = buf_src[1];
                                        buf_des[2] = buf_src[0];

                                        buf_src += 4;
                                        buf_des += 3;
                                    }
                                }

                                pSurface->UnlockRect();

                                bm.SaveInPNG(CWStr(n).Get());
                                g_MatrixMap->m_DI.T(L"Screen shot has been saved", L"");
                            }
                            else
                            {
                                //LockRect fail
                                //OutputDebugStringA("LockRect fail\n");
                            }
                        }
                        else
                        {
                            // GetRenderTargetData fail
                            //char s[256];
                            //sprintf_s(s, sizeof(s), "GetRenderTargetData fail - 0x%08X, %u, %d\n", hr, hr, hr);
                            //OutputDebugStringA(s);
                        }
                        pSurface->Release();
                    }
                    else
                    {
                        //CreateOffscreenPlainSurface fail
                        //OutputDebugStringA("CreateOffscreenPlainSurface fail\n");
                    }
                }
                else
                {
                    // GetDesc fail
                    //OutputDebugStringA("GetDesc fail\n");
                }
            
                if(pTargetSurface) pTargetSurface->Release();
            }
            else
            {
                //GetRenderTarget fail
                //OutputDebugStringA("GetRenderTarget fail\n");
            }

            return;
        }

        CBitmap bm(g_CacheHeap);
        CBitmap bmout(g_CacheHeap);
        bmout.CreateRGB(g_ScreenX, g_ScreenY);

        HDC hdc = GetDC(g_Wnd);

        bm.WBM_Bitmap(CreateCompatibleBitmap(hdc, g_ScreenX, g_ScreenY));
        bm.WBM_BitmapDC(CreateCompatibleDC(hdc));

        if(!SelectObject(bm.WBM_BitmapDC(), bm.WBM_Bitmap()))
        {
            ReleaseDC(g_Wnd, hdc);
            return;
        }

        BitBlt(bm.WBM_BitmapDC(), 0, 0, g_ScreenX, g_ScreenY, hdc, 0, 0, SRCCOPY);

        ReleaseDC(g_Wnd, hdc);

        bm.WBM_Save(true);

        bmout.Copy(CPoint(0, 0), bm.Size(), bm, CPoint(0, 0));
        bmout.SaveInPNG(CWStr(n).Get());
		
        //HFree(data, g_CacheHeap);

        g_MatrixMap->m_DI.T(L"Screen shot has been saved", L"");
    }
}

static int g_LastPosX;
static int g_LastPosY;


#if (defined _DEBUG) &&  !(defined _RELDEBUG)
static SEffectHandler point(DEBUG_CALL_INFO);
static CMatrixEffectPath *path = 0;
static CMatrixEffectSelection *sel = 0;
static CMatrixEffectRepair *repair = 0;

void selcallback(CMatrixMapStatic *ms, DWORD param)
{
    if (ms->GetObjectType() == OBJECT_TYPE_MAPOBJECT) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Mesh",1000);
    else if (ms->IsRobot()) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Robot",1000);
    else if (ms->IsCannon()) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Cannon",1000);
    else if (ms->IsBuilding()) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Building",1000);
    else if (ms->GetObjectType() == OBJECT_TYPE_FLYER) g_MatrixMap->m_DI.T(CWStr((int)ms).Get(), L"Flyer",1000);
    SideSelectionCallBack(ms, param);
}
#else

void selcallback(CMatrixMapStatic *ms, DWORD param)
{
    SideSelectionCallBack(ms, param);
}

#endif


void CFormMatrixGame::MouseMove(int x, int y)
{
    DTRACE();

	CMatrixSideUnit* p_side = g_MatrixMap->GetPlayerSide();

    if (g_MatrixMap->IsMouseCam())
    {
        g_MatrixMap->m_Camera.RotateByMouse(x - g_MatrixMap->m_Cursor.GetPosX(), y - g_MatrixMap->m_Cursor.GetPosY());

        CPoint p = g_MatrixMap->m_Cursor.GetPos();
        ClientToScreen(g_Wnd, &p);

        //SetCursorPos(p.x, p.y);

        if (p_side->GetArcadedObject())
        {
            int dx = x - g_MatrixMap->m_Cursor.GetPosX();
            if (dx < 0) p_side->GetArcadedObject()->AsRobot()->RotateRobotLeft();
            if (dx > 0) p_side->GetArcadedObject()->AsRobot()->RotateRobotRight();
        }

        g_MatrixMap->m_Cursor.SetPos(x,y);

        return;
    }



    g_MatrixMap->m_Cursor.SetPos(x,y);
    p_side->OnMouseMove();


    if (CMultiSelection::m_GameSelection)
    {
        SCallback cbs;
        cbs.mp = CPoint(x, y);
        cbs.calls = 0;
        CMultiSelection::m_GameSelection->Update(g_MatrixMap->m_Cursor.GetPos(), TRACE_ROBOT|TRACE_BUILDING, selcallback, (DWORD)&cbs);
    }

    // interface
    SETFLAG(g_IFaceList->m_IfListFlags, MINIMAP_ENABLE_DRAG);


	//bool fRBut=(GetAsyncKeyState(VK_RBUTTON) & 0x8000)==0x8000;

/*
    { // cell information

        float xx, yy;
        g_MatrixMap->CalcPickVector(CPoint(x,y), vDir);
		g_MatrixMap->UnitPickWorld(g_MatrixMap->GetFrustumCenter(),vDir,&xx,&yy);

        xx /= GLOBAL_SCALE;
        yy /= GLOBAL_SCALE;

        CDText::T("CELL", (CStr((int)xx) + "," + CStr((int)yy)).Get());
    }
*/
#if (defined _DEBUG) &&  !(defined _RELDEBUG)
    if (point.effect)
    {
            D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
            pos.z += 10.0f;
            ((CMatrixEffectPointLight *)point.effect)->SetPos(pos);
    }
    if (sel)
    {
            D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
            pos.z += 10.0f;
            sel->SetPos(pos);
    }
    if (repair)
    {
            D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
            pos.z += 10.0f;
            repair->UpdateData(pos, D3DXVECTOR3(1,0,0));
    }
#endif

}

void CFormMatrixGame::MouseKey(ButtonStatus status, int key, int x, int y)
{
    DTRACE();

    if(status == B_WHEEL)
    {
        while(key > 0)
        {
            g_MatrixMap->m_Camera.ZoomInStep();
            --key;
        }
        while(key < 0)
        {
            g_MatrixMap->m_Camera.ZoomOutStep();
            ++key;
        }

        return;
    }

    DCP();

    if(status == B_UP && key == VK_MBUTTON)
    {
        g_MatrixMap->MouseCam(false);
        return;
    }
    if(status == B_DOWN && key == VK_MBUTTON)
    {
        g_MatrixMap->MouseCam(true);
        //SetCursorPos(g_ScreenX/2, g_ScreenY/2);

        return;
    }


    DCP();

	m_Action = 0;
	
	//bool fCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000;

/*
	if(fCtrl && down && key == VK_RBUTTON)
    {
		D3DXVECTOR3 vpos, vdir;
        g_MatrixMap->CalcPickVector(CPoint(x, y), vdir);
		g_MatrixMap->UnitPickWorld(g_MatrixMap->GetFrustumCenter(), vdir, &m_LastWorldX, &m_LastWorldY);
		m_Action=1;
	}
*/

    DCP();

    //Игрок кликнул левой кнопкой мыши (отпустил), после того, как под его курсором уже сформировалась область выделения
    //Область выделения формируется под курсором даже в случае быстрого однократного клика,
    //однако в выделение в таком случае попадает лишь один объект под курсором
    //Все базовые выделения мышью юнитов и зданий происходят тут
    if(status == B_UP && key == VK_LBUTTON)
    {
    DCP();
        CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
        if(CMultiSelection::m_GameSelection)
        {
            SCallback cbs;
            cbs.mp = CPoint(-1, -1);
            cbs.calls = 0;

            //Завершаем процесс расширения анимации области выделения
            CMultiSelection::m_GameSelection->End();
    DCP();
            if(true/*cbs.calls > 0*/)
            {
    DCP();
                //Если в текущей выделяемой группе имеется либо несколько роботов, либо несколько вертолётов, либо как минимум один робот и один вертолёт
                if(ps->GetCurSelGroup()->GetRobotsCnt() > 1 || ps->GetCurSelGroup()->GetFlyersCnt() > 1 || (ps->GetCurSelGroup()->GetRobotsCnt() + ps->GetCurSelGroup()->GetFlyersCnt()) > 1)
                {
                    ps->GetCurSelGroup()->RemoveBuildings();
                    //Если игрок выделяет юнитов с зажатым Shift
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000 && ps->GetCurGroup())
                    {
                        CMatrixGroupObject* go = ps->GetCurSelGroup()->m_FirstObject;
                        while(go)
                        {
                            if(ps->GetCurGroup()->FindObject(go->GetObject()))
                            {
                                CMatrixGroupObject* go_tmp = go->m_NextObject;
                                ps->RemoveObjectFromSelectedGroup(go->GetObject());
                                go = go_tmp;
                                continue;
                            }
                            go = go->m_NextObject;
                        }
                        ps->AddToCurrentGroup();
                    }
                    else
                    {
                        ps->SetCurGroup(ps->CreateGroupFromCurrent());
                    }
                    if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                    {
                        ps->GetCurGroup()->SortFlyers();
                    }
                    ps->Select(GROUP, NULL);
                }
                //Если в текущей выделяемой группе находится всего один робот
                else if(ps->GetCurSelGroup()->GetRobotsCnt() == 1 && !ps->GetCurSelGroup()->GetFlyersCnt())
                {
    DCP();
                    ps->GetCurSelGroup()->RemoveBuildings();

                    //Если игрок выделяет юнитов с зажатым Shift
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000 && ps->GetCurGroup())
                    {
                        if(ps->GetCurGroup()->FindObject(ps->GetCurSelGroup()->m_FirstObject->GetObject()))
                        {
                            ps->RemoveObjectFromSelectedGroup(ps->GetCurSelGroup()->m_FirstObject->GetObject());
                        }
                        else
                        {
                            ps->AddToCurrentGroup();
                        }

                        if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                        {
                            ps->GetCurGroup()->SortFlyers();
                        }
                        ps->Select(GROUP, NULL);
                    }
                    else
                    {
                        ps->SetCurGroup(ps->CreateGroupFromCurrent());
                        ps->Select(ROBOT, NULL);
                    }
                }
                //Если в текущей выделяемой группе находится всего один вертолёт
                else if(ps->GetCurSelGroup()->GetFlyersCnt() == 1 && !ps->GetCurSelGroup()->GetRobotsCnt())
                {
                    DCP();
                    ps->GetCurSelGroup()->RemoveBuildings();
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_SHIFT]) & 0x8000) == 0x8000 && ps->GetCurGroup())
                    {
                        if(ps->GetCurGroup()->FindObject(ps->GetCurSelGroup()->m_FirstObject->GetObject()))
                        {
                            ps->RemoveObjectFromSelectedGroup(ps->GetCurSelGroup()->m_FirstObject->GetObject());
                        }
                        else
                        {
                            ps->AddToCurrentGroup();
                        }
                        if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                        {
                            ps->GetCurGroup()->SortFlyers();
                        }
                        ps->Select(GROUP, NULL);
                    }
                    else
                    {
                        ps->SetCurGroup(ps->CreateGroupFromCurrent());
                        ps->Select(FLYER, NULL);
                    }
                }
                //Если в текущей выделяемой группе находится здание
                else if(ps->GetCurSelGroup()->GetBuildingsCnt() && !ps->GetCurSelGroup()->GetRobotsCnt() && !ps->GetCurSelGroup()->GetFlyersCnt())
                {
    DCP();
                    ps->Select(BUILDING, ps->GetCurSelGroup()->m_FirstObject->GetObject());
                    ps->GroupsUnselectSoft();
                    ps->GetCurSelGroup()->RemoveAll();
                    ps->SetCurGroup(NULL);
                    ps->Reselect();
                }
            }
        }
        CMultiSelection::m_GameSelection = NULL;
	}
    DCP();

    if(g_IFaceList->m_InFocus == INTERFACE)
    {
    DCP();
		if(status == B_UP && key == VK_LBUTTON)
        {
			g_IFaceList->OnMouseLBUp();
		}
        else if((status == B_DOWN || status == B_DOUBLE) && key == VK_LBUTTON)
        {
            g_IFaceList->OnMouseLBDown();
		}
        if(status == B_DOWN && key == VK_RBUTTON)
        {
            g_IFaceList->OnMouseRBDown();
        }
	}
    else if(g_IFaceList->m_InFocus == UNKNOWN) //or something else
    {
    DCP();
		if(status == B_DOWN && key == VK_RBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnRButtonDown(CPoint(x, y));
		}
        else if(status == B_DOWN && key==VK_LBUTTON)
        {
    DCP();
            //Если игрок зажал левую кнопку мыши и расширяет область выделения
            if(CMultiSelection::m_GameSelection == NULL && !g_MatrixMap->GetPlayerSide()->IsArcadeMode() && !IS_PREORDERING_NOSELECT && !(g_MatrixMap->GetPlayerSide()->m_CurrentAction == BUILDING_TURRET))
            {
                int dx = 0, dy = 0;
                if(IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj) && IS_TRACE_UNIT(g_MatrixMap->m_TraceStopObj))
                {
                    dx = 2;
                    dy = 2;
                }
                CMultiSelection::m_GameSelection = CMultiSelection::Begin(CPoint(g_MatrixMap->m_Cursor.GetPos().x - dx, g_MatrixMap->m_Cursor.GetPos().y - dy));
                if(CMultiSelection::m_GameSelection)
                {
                    SCallback cbs;
                    cbs.mp = g_MatrixMap->m_Cursor.GetPos();
                    cbs.calls = 0;

                    CMultiSelection::m_GameSelection->Update(g_MatrixMap->m_Cursor.GetPos(), TRACE_ROBOT | TRACE_BUILDING, selcallback, (DWORD)&cbs);
                }
            }
            g_MatrixMap->GetPlayerSide()->OnLButtonDown(CPoint(x, y));
		}
        else if(status == B_UP && key == VK_RBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnRButtonUp(CPoint(x, y));
        }
        else if(status == B_UP && key == VK_LBUTTON)
        {
    DCP();
    		CMatrixSideUnit* ps = g_MatrixMap->GetPlayerSide();
            ps->OnLButtonUp(CPoint(x, y));
		}
        else if(status == B_DOUBLE && key == VK_LBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnLButtonDouble(CPoint(x, y));
		}
        else if(status == B_DOUBLE && key == VK_RBUTTON)
        {
    DCP();
			g_MatrixMap->GetPlayerSide()->OnRButtonDouble(CPoint(x, y));
		}
	}
}

void ExitRequestHandler(void);
void ConfirmCancelHandler(void);
void ResetRequestHandler(void);
void SurrenderRequestHandler(void);

struct STextInfo
{
    const wchar *t1;
    const wchar *t2;
    int time;
};

static STextInfo stuff[] = 
{
    {L"3D Robots game information....", L"", 3000},
    {L"Coding....", L"", 10000},
    {L" Alexander <ZakkeR> Zeberg", L"", 0},
    {L"", L"Engine lead coder", 0},
    {L"", L"MapEditor lead coder"},
    {L"", L"Optimizations"},
    {L" Alexey <Dab> Dubovoy", L"", 0},
    {L"", L"High-AI lead coder", 0},
    {L"", L"MapEditor base coder", 0},
    {L" Alexander <Sub0> Parshin", L"", 0},
    {L"", L"Low-AI lead coder", 0},
    {L"", L"UI lead coder", 0},

    {L"Artwork...", L"", 10000},
    {L" Eugene <Johan> Cherenkov", L"", 0},
    {L"", L"UI", 0},
    {L"", L"Some cool textures", 0},
    {L"", L"Sky", 0},

    {L" Nina <Nina> Vatulich", L"", 0},
    {L"", L"Terrain textures", 0},
    {L"", L"Effects textures", 0},

    {L"Modeling...", L"", 8000},
    {L" Nina <Nina> Vatulich", L"", 0},
    {L"", L"Lot of meshes", 0},

    {L" Alexander <Alexartist> Yazynin", L"", 0},
    {L"", L"Meshes", 0},
    {L"", L"Buildings", 0},

    {L" Ruslan <IronFist> Tchernyi", L"", 0},
    {L"", L"Advanced meshes", 0},

    {L" Sergey <Esk> Simonov", L"", 0},
    {L"", L"Robots", 0},
    {L"", L"Helicopters", 0},
    {L"", L"Some meshes", 0},

    {L"Map design...", L"", 7000},
    {L" Alexander <Alexartist> Yazynin", L"", 0},
    {L" Ruslan <IronFist> Tchernyi", L"", 0},
    {L" Nina <Nina> Vatulich", L"", 0},


    {L"Game balancing...", L"", 7000},
    {L" Alexander <Alexartist> Yazynin", L"", 0},
    {L"", L"Maps", 0},
    {L" Dmitry <Dm> Gusarov", L"", 0},
    {L"", L"Items", 0},
    {L" Nina <Nina> Vatulich", L"", 0},
    {L"", L"Maps", 0},

    {L"Game texts and sounds...", L"", 5000},
    {L" Ilia <Ilik> Plusnin", L"", 0},

    {L"Thats all folks :)", L"", 3000},
    {NULL, NULL}
};

//Обработчики нажатия кнопок на клавиатуре
void CFormMatrixGame::Keyboard(bool down, int scan)
{
    DTRACE();

    bool fCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000;

    //Если была активирована дев-консоль, то обработка всех прочих нажатий отрубается
    //С активной консолью все нажатия должен начать перехватывать код консоли в DevConsole.cpp
    if(g_MatrixMap->m_Console.IsActive())
    {
        //g_MatrixMap->m_Console.SetActive(true);
        g_MatrixMap->m_Console.Keyboard(scan, down);
        return;
    }

    if(down)
    {
        memcpy(&m_LastScans[0], &m_LastScans[1], sizeof(SKeyScan) * (MAX_SCANS - 1));
        m_LastScans[MAX_SCANS - 1].scan = scan;
        m_LastScans[MAX_SCANS - 1].time = g_MatrixMap->GetTime();

//Обработчики набора читов
#ifdef CHEATS_ON
//Вызов консоли не срабатывает, нужно откапывать оригинальную функцию вызова
/*
        if (m_LastScans[MAX_SCANS - 6].scan == KEY_D)
            if (m_LastScans[MAX_SCANS - 5].scan == KEY_E)
                if (m_LastScans[MAX_SCANS - 4].scan == KEY_V)
                    if (m_LastScans[MAX_SCANS - 3].scan == KEY_C)
                        if (m_LastScans[MAX_SCANS - 2].scan == KEY_O)
                            if (m_LastScans[MAX_SCANS - 1].scan == KEY_N)
                            //if (m_LastScans[MAX_SCANS - 1].scan == KEY_TILDA)
                            {
                                m_LastScans[MAX_SCANS - 1].scan = 0;
                                g_MatrixMap->m_Console.SetActive(true);
                                return;
                            }
*/
        if (m_LastScans[MAX_SCANS - 7].scan == KEY_S)
            if (m_LastScans[MAX_SCANS - 6].scan == KEY_H)
                if (m_LastScans[MAX_SCANS - 5].scan == KEY_O)
                    if (m_LastScans[MAX_SCANS - 4].scan == KEY_W)
                        if (m_LastScans[MAX_SCANS - 3].scan == KEY_F)
                            if (m_LastScans[MAX_SCANS - 2].scan == KEY_P)
                                if (m_LastScans[MAX_SCANS - 1].scan == KEY_S)
                                {
                                    m_LastScans[MAX_SCANS - 1].scan = 0;
                                    INVERTFLAG(g_Config.m_DIFlags, DI_DRAWFPS);
                                    return;
                                }
        if (m_LastScans[MAX_SCANS - 5].scan == KEY_I)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_N)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_F)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_O)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_SPACE)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            INVERTFLAG(g_Config.m_DIFlags, DI_TMEM|DI_TARGETCOORD|DI_VISOBJ|DI_ACTIVESOUNDS|DI_FRUSTUMCENTER);
                            return;
                        }
        if (m_LastScans[MAX_SCANS - 4].scan == KEY_A)
            if (m_LastScans[MAX_SCANS - 3].scan == KEY_U)
                if (m_LastScans[MAX_SCANS - 2].scan == KEY_T)
                    if (m_LastScans[MAX_SCANS - 1].scan == KEY_O)
                    {
                        m_LastScans[MAX_SCANS - 1].scan = 0;
                        INVERTFLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE);
                        return;
                    }

        if (m_LastScans[MAX_SCANS - 6].scan == KEY_F)
            if (m_LastScans[MAX_SCANS - 5].scan == KEY_L)
                if (m_LastScans[MAX_SCANS - 4].scan == KEY_Y)
                    if (m_LastScans[MAX_SCANS - 3].scan == KEY_C)
                        if (m_LastScans[MAX_SCANS - 2].scan == KEY_A)
                            if (m_LastScans[MAX_SCANS - 1].scan == KEY_M)
                            {
                                m_LastScans[MAX_SCANS - 1].scan = 0;
                                INVERTFLAG(g_MatrixMap->m_Flags, MMFLAG_FLYCAM);
                                return;
                            }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_B)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_A)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_B)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_K)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_I)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            INVERTFLAG(g_Config.m_DIFlags, DI_SIDEINFO);
                            return;
                        }

        if (m_LastScans[MAX_SCANS - 8].scan == KEY_C)
            if (m_LastScans[MAX_SCANS - 7].scan == KEY_R)
                if (m_LastScans[MAX_SCANS - 6].scan == KEY_A)
                    if (m_LastScans[MAX_SCANS - 5].scan == KEY_Z)
                        if (m_LastScans[MAX_SCANS - 4].scan == KEY_Y)
                            if (m_LastScans[MAX_SCANS - 3].scan == KEY_B)
                                if (m_LastScans[MAX_SCANS - 2].scan == KEY_O)
                                    if (m_LastScans[MAX_SCANS - 1].scan == KEY_T)
                                    {
                                        m_LastScans[MAX_SCANS - 1].scan = 0;
                                        g_MatrixMap->GetPlayerSide()->BuildCrazyBot();
                                        return;
                                    }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_H)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_U)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_R)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_R)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_Y)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            if (!g_MatrixMap->MaintenanceDisabled() && g_MatrixMap->BeforeMaintenanceTime() > 0)
                            {
                                g_MatrixMap->SetMaintenanceTime(1);
                                return;
                            }

                        }

        if (m_LastScans[MAX_SCANS - 8].scan == KEY_M)
            if (m_LastScans[MAX_SCANS - 7].scan == KEY_E)
                if (m_LastScans[MAX_SCANS - 6].scan == KEY_G)
                    if (m_LastScans[MAX_SCANS - 5].scan == KEY_A)
                        if (m_LastScans[MAX_SCANS - 4].scan == KEY_B)
                            if (m_LastScans[MAX_SCANS - 3].scan == KEY_U)
                                if (m_LastScans[MAX_SCANS - 2].scan == KEY_S)
                                    if (m_LastScans[MAX_SCANS - 1].scan == KEY_T)
                                    {
                                        m_LastScans[MAX_SCANS - 1].scan = 0;
                            
                                        //CSound::Play(S_BENTER, SL_INTERFACE);
                                        //CSound::Play(S_MAINTENANCE_ON, SL_INTERFACE);
                                        //CSound::Play(S_ROBOT_BUILD_END, SL_ALL);
                                        //CSound::Play(S_TERRON_KILLED, SL_ALL);
                                        //CSound::Play(S_TURRET_BUILD_2, SL_ALL);

                                        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_MEGABUSTALREADY))
                                        //if(0)
                                        {
                                            CMatrixMapStatic* s = CMatrixMapStatic::GetFirstLogic();
                                            for(; s; s = s->GetNextLogic())
                                            {
                                                if(s->GetSide() == PLAYER_SIDE)
                                                {
                                                    if(s->IsRobot() && !s->AsRobot()->IsAutomaticMode())
                                                    {
                                                        s->AsRobot()->MustDie();
                                                    }
                                                    //else if(s->IsCannon())
                                                    //{
                                                    //    s->AsCannon()->InitMaxHitpoint(s->AsCannon()->GetMaxHitPoint() * 20);
                                                    //}
                                                    //else if(s->IsBuilding())
                                                    //{
                                                    //    s->AsBuilding()->InitMaxHitpoint(s->AsBuilding()->GetMaxHitPoint() * 20);
                                                    //}

                                                }
                                            }


                                        }
                                        else
                                        {
                                            CMatrixMapStatic* s = CMatrixMapStatic::GetFirstLogic();
                                            for(; s; s = s->GetNextLogic())
                                            {
                                                if (s->GetSide() == PLAYER_SIDE)
                                                {
                                                    if (s->IsRobot())
                                                    {
                                                        s->AsRobot()->InitMaxHitpoint(s->AsRobot()->GetMaxHitPoint() * 20);
                                            
                                                    }
                                                    else if (s->IsCannon())
                                                    {
                                                        s->AsCannon()->InitMaxHitpoint(s->AsCannon()->GetMaxHitPoint() * 20);
                                                    }
                                                    else if (s->IsBuilding())
                                                    {
                                                        s->AsBuilding()->InitMaxHitpoint(s->AsBuilding()->GetMaxHitPoint() * 20);
                                                    }

                                                }
                                            }
                                            SETFLAG(g_MatrixMap->m_Flags, MMFLAG_MEGABUSTALREADY);
                                        }
                                        return;
                                    }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_S)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_T)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_A)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_T)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_S)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;

                            if (!g_MatrixMap->IsPaused())
                            {
                                SETFLAG(g_MatrixMap->m_Flags, MMFLAG_STAT_DIALOG_D);
                            }
                            return;
                        }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_V)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_I)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_D)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_E)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_O)
                        {
                            m_LastScans[MAX_SCANS - 1].scan = 0;
                            g_MatrixMap->m_DI.T(L"_____________________________", L"",10000);
                            g_MatrixMap->m_DI.T(L"Sim textures", CWStr(g_D3DDCaps.MaxSimultaneousTextures, g_CacheHeap).Get(), 10000);
                            g_MatrixMap->m_DI.T(L"Stencil available", FLAG(g_Flags,GFLAG_STENCILAVAILABLE)?L"Yes":L"No", 10000);

                            // vidmode
	                        D3DDISPLAYMODE d3ddm;

                            CWStr modet(g_CacheHeap);
                            CWStr modev(g_CacheHeap);
                            for (int i = 0; i < 2; ++i)
                            {
                                modet.Set(L"Buffer ");
                                modet += i;
                                modet += L" mode";
	                            ASSERT_DX(g_D3DD->GetDisplayMode(0,&d3ddm));
                                if (d3ddm.Format == D3DFMT_X8R8G8B8)
                                {
                                    modev = L"X8R8G8B8";
                                }
                                else if (d3ddm.Format == D3DFMT_A8R8G8B8)
                                {
                                    modev = L"A8R8G8B8";
                                }
                                else if (d3ddm.Format == D3DFMT_R8G8B8)
                                {
                                    modev = L"R8G8B8";
                                }
                                else if (d3ddm.Format == D3DFMT_R5G6B5)
                                {
                                    modev = L"R5G6B5";
                                }
                                else
                                {
                                    modev.Set(d3ddm.Format);
                                }
                                g_MatrixMap->m_DI.T(modet.Get(), modev.Get(), 10000);

                            }

                            return;
                        }

        if (m_LastScans[MAX_SCANS - 9].scan == KEY_I)
            if (m_LastScans[MAX_SCANS - 8].scan == KEY_A)
                if (m_LastScans[MAX_SCANS - 7].scan == KEY_M)
                    if (m_LastScans[MAX_SCANS - 6].scan == KEY_L)
                        if (m_LastScans[MAX_SCANS - 5].scan == KEY_O)
                            if (m_LastScans[MAX_SCANS - 4].scan == KEY_O)
                                if (m_LastScans[MAX_SCANS - 3].scan == KEY_S)
                                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_E)
                                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_R)
                                        {
                                            g_ExitState=3;
                                            g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_WIN);
                                            return;
                                        }

        if (m_LastScans[MAX_SCANS - 7].scan == KEY_B)
            if (m_LastScans[MAX_SCANS - 6].scan == KEY_U)
                if (m_LastScans[MAX_SCANS - 5].scan == KEY_B)
                    if (m_LastScans[MAX_SCANS - 4].scan == KEY_U)
                        if (m_LastScans[MAX_SCANS - 3].scan == KEY_B)
                            if (m_LastScans[MAX_SCANS - 2].scan == KEY_U)
                                if (m_LastScans[MAX_SCANS - 1].scan == KEY_SPACE)
                                {
                                    m_LastScans[MAX_SCANS - 1].scan = 0;

                                    int delay = 0;
                                    int ctime = 0;
                                    int od = 0;
                                    for (int i = 0; stuff[i].t1 != NULL; ++i)
                                    {
                                        if (stuff[i].time)
                                        {
                                            od = delay;
                                            ctime = stuff[i].time;
                                            delay += stuff[i].time + 100;
                                        }
                                        g_MatrixMap->m_DI.T(stuff[i].t1, stuff[i].t2, ctime, od, true);
                                    }
                                    return;
                                }

        if (m_LastScans[MAX_SCANS - 5].scan == KEY_C)
            if (m_LastScans[MAX_SCANS - 4].scan == KEY_R)
                if (m_LastScans[MAX_SCANS - 3].scan == KEY_A)
                    if (m_LastScans[MAX_SCANS - 2].scan == KEY_S)
                        if (m_LastScans[MAX_SCANS - 1].scan == KEY_H)
                        {
                            _asm xor eax,eax
                            _asm mov [eax],  eax
                            return;
                        }
#endif
    }

    //Горячие клавиши для команд в игровом меню (выйти, рестарт, сдаться и т.д.)
    if(scan == KEY_ENTER && down)
    {
        if(g_MatrixMap->m_DialogModeName && (g_MatrixMap->m_DialogModeHints.Len() > 4 || wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) != 0))
        {
            g_IFaceList->PressHintButton(HINT_OK);
            return;
        }
    } 

    if(scan == KEY_E && down)
    {
        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {}
            else
            {
                ExitRequestHandler();
                return;
            }
        }
    } 
    if(scan == KEY_S && down)
    {
        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {}
            else
            {
                SurrenderRequestHandler();
                return;
            }
        }
    } 
    if(scan == KEY_R && down)
    {
        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {}
            else
            {
                ResetRequestHandler();
                return;
            }
        }
    } 

    if(scan == KEY_ESC && down)
    {
        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_FULLAUTO))
        {
            g_ExitState = 1;
            SETFLAG(g_Flags, GFLAG_EXITLOOP);

            return;
        }

        if(g_MatrixMap->m_DialogModeName && wcscmp(g_MatrixMap->m_DialogModeName, TEMPLATE_DIALOG_MENU) == 0)
        {
            if(g_MatrixMap->m_DialogModeHints.Len() > 4)
            {
                ConfirmCancelHandler();
            }
            else
            {
                g_MatrixMap->LeaveDialogMode();
            }
            return;
        }

        if(FLAG(g_MatrixMap->m_Flags, MMFLAG_DIALOG_MODE)) return;

        if(g_MatrixMap->GetPlayerSide()->GetArcadedObject())
        {
            g_IFaceList->LeaveRobot();
            return;
        }

#ifdef _TRACE
        if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0x8000)
        {
            g_ExitState=2;
            //SETFLAG(g_Flags, GFLAG_EXITLOOP);
            g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_LOOSE);
            return;
        }
        if((GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0x8000)
        {
            g_ExitState=3;
            //SETFLAG(g_Flags, GFLAG_EXITLOOP);
            g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_WIN);
            return;
        }
#endif
        g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_MENU);
		return;
	}

    //down - однократное нажатие кнопки (точнее какая конкретно кнопка была утоплена)
    //scan - запись последовательности нажатых за последнее время кнопок
    g_MatrixMap->m_KeyDown = down;
    g_MatrixMap->m_KeyScan = scan;

    if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_FORWARD_ALT]) & 0x8000) == 0x8000))
    {
        g_MatrixMap->GetPlayerSide()->OnForward(true);
    }
    if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BACKWARD_ALT]) & 0x8000) == 0x8000))
    {
        g_MatrixMap->GetPlayerSide()->OnBackward(true);
    }

    if(down)
    {
        //Выставление камеры в позицию по умолчанию
        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_CAM_SETDEFAULT]) & 0x8000) == 0x8000)
        {
            g_MatrixMap->m_Camera.ResetAngles();
            return;
        }

        //Стандартный скриншот с занесением изображения в буфер обмена
        //Не пиняйте строго, код взял на stackoverflow
        //К тому же оно не сохраняет эффекты на экране, вообще хз (Klaxons)
        if(GetAsyncKeyState(g_Config.m_KeyActions[KA_TAKE_SCREENSHOT]))//if(scan == KEY_SNAPSHOT)
        {
            int x1, y1, x2, y2, w, h;

            // get screen dimensions
            x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
            y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
            x2 = GetSystemMetrics(SM_CXVIRTUALSCREEN);
            y2 = GetSystemMetrics(SM_CYVIRTUALSCREEN);
            w = x2 - x1;
            h = y2 - y1;

            // copy screen to bitmap
            HDC     hScreen = GetDC(NULL);
            HDC     hDC = CreateCompatibleDC(hScreen);
            HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
            HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
            BOOL    bRet = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

            // save bitmap to clipboard
            OpenClipboard(NULL);
            EmptyClipboard();
            SetClipboardData(CF_BITMAP, hBitmap);
            CloseClipboard();

            // clean up
            SelectObject(hDC, old_obj);
            DeleteDC(hDC);
            ReleaseDC(NULL, hScreen);
            DeleteObject(hBitmap);

            return;
        }

        //Выставление паузы :|
        if(GetAsyncKeyState(g_Config.m_KeyActions[KA_GAME_PAUSED]))//if(scan == KEY_PAUSE)
        {
            g_MatrixMap->Pause(!g_MatrixMap->IsPaused());
            return;
        }

        CMatrixSideUnit *ps = g_MatrixMap->GetPlayerSide();
        if(ps->IsRobotMode())
        {
            CMatrixRobotAI *robot = ps->GetArcadedObject()->AsRobot();

            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_BOOM]) & 0x8000) == 0x8000)
            {
                //"E" - Взорвать.
                robot->BigBoom(); 
                return;
            }
            if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER_ALT]) & 0x8000) == 0x8000))
            {
                //"Esc", "Пробел", "Enter" - выйти из режима ручного управления роботом
                if(!robot->IsManualControlLocked()) g_IFaceList->LeaveRobot();
                return;
            }
        }
        else
        {
            //Выбор всех роботов игрока на карте
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ALL_UNITS_SELECT]) & 0x8000) == 0x8000)//if(scan == KEY_F2)
            {
                CMatrixMapStatic *o = CMatrixMapStatic::GetFirstLogic();
                if(ps->GetCurGroup())
                {
                    ps->SelectedGroupUnselect();
                    ps->GetCurSelGroup()->RemoveAll();
                }

                while(o)
                {
                    if(o->GetSide() == PLAYER_SIDE && o->IsLiveRobot() && !o->AsRobot()->IsCrazy())
                    {
                        ps->GetCurSelGroup()->AddObject(o, -4);
                    }
                    o = o->GetNextLogic();
                }

                ps->CreateGroupFromCurrent();

                if(ps->GetCurGroup() && ps->GetCurGroup()->GetObjectsCnt() == 1)
                {
                    ps->Select(ROBOT, NULL);
                }
                else if(ps->GetCurGroup() && ps->GetCurGroup()->GetObjectsCnt() > 1)
                {
                    ps->Select(GROUP, NULL);
                }

                return;
            }

            if(!FLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE)/*!IS_PREORDERING_NOSELECT*/)
            {
                //Если мы не в режиме приказа
                if(ps->GetCurGroup() && (ps->m_CurrSel == ROBOT_SELECTED || ps->m_CurrSel == GROUP_SELECTED))
                {
                    //Стратегический режим - выбрана группа
                    if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER]) & 0x8000) == 0x8000) || ((GetAsyncKeyState(g_Config.m_KeyActions[KA_UNIT_ENTER_ALT]) & 0x8000) == 0x8000))
                    {
                        //"Esc", "Пробел", "Enter" - войти в режим ручного управления роботом
                        CMatrixMapStatic* obj = ps->GetCurGroup()->GetObjectByN(ps->GetCurSelNum());
                        if(!obj->IsManualControlLocked())
                        {
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->CreateGroupFromCurrent(obj);
                            g_IFaceList->EnterRobot(false);
                        }
                    }
                    else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTOORDER_ATTACK]) & 0x8000) == 0x8000))
                    {
                        //a"U"to attack - Программа атаки.
                        if(FLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON))
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON|AUTO_CAPTURE_ON|AUTO_PROTECT_ON);
                            ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        }
                        else
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON|AUTO_CAPTURE_ON|AUTO_PROTECT_ON);
                            SETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON);
                            ps->PGOrderAutoAttack(ps->SelGroupToLogicGroup());
                        }
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTOORDER_CAPTURE]) & 0x8000) == 0x8000)
                    {
                        //"C"apture - Программа захвата.
                        if(FLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON))
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON|AUTO_CAPTURE_ON|AUTO_PROTECT_ON);
                            ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        }
                        else
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON|AUTO_CAPTURE_ON|AUTO_PROTECT_ON);
                            SETFLAG(g_IFaceList->m_IfListFlags, AUTO_CAPTURE_ON);
                            ps->PGOrderAutoCapture(ps->SelGroupToLogicGroup());
                        }
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_AUTOORDER_DEFEND]) & 0x8000) == 0x8000)
                    {
                        //"D"efender - Программа Охранять Protect
                        if(FLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON))
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON|AUTO_CAPTURE_ON|AUTO_PROTECT_ON);
                            ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        }
                        else
                        {
                            RESETFLAG(g_IFaceList->m_IfListFlags, AUTO_FROBOT_ON|AUTO_CAPTURE_ON|AUTO_PROTECT_ON);
                            SETFLAG(g_IFaceList->m_IfListFlags, AUTO_PROTECT_ON);
                            ps->PGOrderAutoDefence(ps->SelGroupToLogicGroup());
                        }
                    }
                    else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_MOVE]) & 0x8000) == 0x8000))
                    {
                        //"M"ove - Двигаться
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_MOVE);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_STOP]) & 0x8000) == 0x8000))
                    {
                        //"S"top - Стоять
                        ps->PGOrderStop(ps->SelGroupToLogicGroup());
                        ps->SelectedGroupBreakOrders();
                    }
                    else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_CAPTURE]) & 0x8000) == 0x8000))
                    {
                        //Ta"K"e - Захватить
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_CAPTURE);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_PATROL]) & 0x8000) == 0x8000))
                    {
                        //"P"atrol - Патрулировать
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_PATROL);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if(ps->GetCurGroup()->GetBombersCnt() && ((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_EXPLODE]) & 0x8000) == 0x8000))
                    {
                        //"E"xplode - Взорвать
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BOMB);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if(ps->GetCurGroup()->GetRepairsCnt() && ((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_REPAIR]) & 0x8000) == 0x8000))
                    {
                        //"R"epair - Чинить
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_REPAIR);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_ATTACK]) & 0x8000) == 0x8000)
                    {
                        //"A"ttack
                        SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_FIRE);
                        SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);        
                    }
                }
                else if(ps->m_CurrSel == BUILDING_SELECTED || ps->m_CurrSel == BASE_SELECTED)
                {
                    //Стратегический режим - выбрана база или завод
                    CMatrixBuilding* bld = (CMatrixBuilding*)ps->m_ActiveObject;
                    
                    //Быстрые клавиши для базы и заводов
                    if(!bld->IsBase() || !ps->m_ConstructPanel->IsActive())
                    {
                        if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_TURRET]) & 0x8000) == 0x8000)
                        {
                            //"T"urrel - переход в меню выбора турели
                            CPoint pl[MAX_PLACES];
                            bool cant_build_tu = false;

                            if((ps->IsEnoughResources(g_Config.m_CannonsProps[0].m_Resources) ||
                                ps->IsEnoughResources(g_Config.m_CannonsProps[1].m_Resources) ||
                                ps->IsEnoughResources(g_Config.m_CannonsProps[2].m_Resources) ||
                                ps->IsEnoughResources(g_Config.m_CannonsProps[3].m_Resources)) &&
                                (bld->GetPlacesForTurrets(pl) > 0) && (!bld->m_BS.IsMaxItems()))
                            {
                                ps->m_ConstructPanel->ResetGroupNClose();
                                SETFLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE);
                                SETFLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET);

                                CMatrixMapStatic* ms = CMatrixMapStatic::GetFirstLogic();
                                for(; ms; ms = ms->GetNextLogic())
                                {
                                    if(ms == ps->m_ActiveObject && ms->IsLiveBuilding() && ms->AsBuilding()->m_Side == PLAYER_SIDE)
                                    {
                                        ms->AsBuilding()->CreatePlacesShow();
                                        break;
                                    }
                                }
                            }
                        }
                        else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_HELP]) & 0x8000) == 0x8000)
                        {
                            //"Н"elp - Вызов подкрепления
                            bld->Maintenance();
                        }
                    }

                    //Быстрые клавиши только для базы
                    if(bld->IsBase())
                    {
                        //Игрок выбрал базу и находится на её общем экране
                        if(!ps->m_ConstructPanel->IsActive())
                        {
                            //"B"uild - переход в режим конструктора роботов
                            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT]) & 0x8000) == 0x8000)
                            {
                                g_IFaceList->m_RCountControl->Reset();
                                g_IFaceList->m_RCountControl->CheckUp();
                                ps->m_ConstructPanel->ActivateAndSelect();
                            }
                            //"G"athering point - установка точки сбора базы
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_GATHERING_POINT]) & 0x8000) == 0x8000)
                            {
                                bld->SetGatheringPoint(Float2Int(g_MatrixMap->m_TraceStopPos.x), Float2Int(g_MatrixMap->m_TraceStopPos.y));
                                //Первоначальная отрисовка точки сбора (перенесена в функцию общей постоянной отрисовки)
                                //CMatrixEffect::CreateMoveto(2);
                            }
                        }
                        //Игрок находится в режиме конструктора роботов выбранной базы
                        else
                        {
                            //"B"uild (либо "X") - выход из режима конструктора роботов
                            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT]) & 0x8000) == 0x8000 || (GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_CANCEL]) & 0x8000) == 0x8000)
                            {
                                ps->m_ConstructPanel->ResetGroupNClose();
                            }
                            //"Enter" - запуск производства робота
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_START]) & 0x8000) == 0x8000)
                            {
                                if(g_MatrixMap->GetPlayerSide()->m_Constructor->m_BaseConstBuildBut->GetState() != IFACE_DISABLED)
                                {
                                    CSound::Play(S_BUILD_CLICK, SL_INTERFACE);
                                    g_MatrixMap->GetPlayerSide()->m_Constructor->RemoteBuild(NULL);
                                }
                            }
                            //"Arrow Up" - увеличение числа роботов для постройки
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_QUANTITY_UP]) & 0x8000) == 0x8000)
                            {
                                if(g_IFaceList->m_RCountControl->m_ButtonUp->GetState() != IFACE_DISABLED)
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_IFaceList->m_RCountControl->Up(NULL);
                                }
                            }
                            //"Arrow Down" - снижение числа роботов для постройки
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_QUANTITY_DOWN]) & 0x8000) == 0x8000)
                            {
                                if(g_IFaceList->m_RCountControl->m_ButtonDown->GetState() != IFACE_DISABLED)
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_IFaceList->m_RCountControl->Down(NULL);
                                }
                            }
                            //"Arrow Left" - выбор схемы робота для постройки влево
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_CHOOSE_LEFT]) & 0x8000) == 0x8000)
                            {
                                if(g_ConfigHistory->IsPrev())
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_ConfigHistory->PrevConfig(NULL);
                                }
                            }
                            //"Arrow Right" - выбор схемы робота для постройки вправо
                            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_ROBOT_CHOOSE_RIGHT]) & 0x8000) == 0x8000)
                            {
                                if(g_ConfigHistory->IsNext())
                                {
                                    CSound::Play(S_BCLICK, SL_INTERFACE);
                                    g_ConfigHistory->NextConfig(NULL);
                                }
                            }
                            //Были нажаты цифровые клавиши 1-9 (по местной нумерации 2-10)
                            else if(scan > 1 && scan <= 11 && ps->m_ConstructPanel->m_FocusedElement)
                            {
                                int key = 0;
                                if (scan != 11) key = scan - 1;

                                ERobotUnitType type = MRT_EMPTY;
                                ERobotUnitKind kind = RUK_UNKNOWN;
                                int pilon = -1;

                                //RUK_WEAPON_MACHINEGUN      = 1,  1
                                //RUK_WEAPON_CANNON          = 2,  2
                                //RUK_WEAPON_MISSILE         = 3,  3
                                //RUK_WEAPON_FLAMETHROWER    = 4,  4
                                //RUK_WEAPON_MORTAR          = 5,
                                //RUK_WEAPON_LASER           = 6,  5
                                //RUK_WEAPON_BOMB            = 7,
                                //RUK_WEAPON_PLASMA          = 8,  6
                                //RUK_WEAPON_ELECTRIC        = 9,  7
                                //RUK_WEAPON_REPAIR          = 10, 8

                                if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON1)
                                {
                                    pilon = 0;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON2)
                                {
                                    pilon = 1;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON3)
                                {
                                    pilon = 2;
                                    type = MRT_WEAPON;
                                }
                                else if(ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON4)
                                {
                                    pilon = 3;
                                    type = MRT_WEAPON;
                                }
                                else if(key <= 2 && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON5)
                                {
                                    pilon = 4;
                                    type = MRT_WEAPON;

                                    kind = ERobotUnitKind(key);

                                    if(key == 1) kind = RUK_WEAPON_MORTAR;
                                    else if(key == 2) kind = RUK_WEAPON_BOMB;
                                }
                                else if(key <= 4 && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON_HEAD)
                                {
                                    pilon = 0;
                                    type = MRT_HEAD;

                                    kind = ERobotUnitKind(key);
                                }
                                else if(key && key <= 6 && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON_HULL)
                                {
                                    pilon = 0;
                                    type = MRT_ARMOR;

                                    if(key == 1) kind = ERobotUnitKind(6);
                                    else kind = ERobotUnitKind(key - 1);
                                }
                                else if(key && key <= 5 && ps->m_ConstructPanel->m_FocusedElement->m_strName == IF_BASE_PILON_CHASSIS)
                                {
                                    pilon = 0;
                                    type = MRT_CHASSIS;
                                    kind = ERobotUnitKind(key);
                                }

                                if(key <= 8 && type == MRT_WEAPON && pilon < 4)
                                {
                                    if(key == 0) kind = ERobotUnitKind(0);
                                    else if(key == 5) kind = ERobotUnitKind(key + 1);
                                    else if(key > 5) kind = ERobotUnitKind(key + 2);
                                    else kind = ERobotUnitKind(key);
                                }

                                if (int(type)) ps->m_Constructor->SuperDjeans(type, kind, pilon);
                            }
                        }
                    }
                }
            }
            else
            {
                //Если мы в режиме приказа или постройки пушки
                if(FLAG(g_IFaceList->m_IfListFlags, ORDERING_MODE) && FLAG(g_IFaceList->m_IfListFlags, PREORDER_BUILD_TURRET) && ps->m_CurrentAction != BUILDING_TURRET)
                {
                    //player_side->IsEnoughResources(g_Config.m_CannonsProps[1].m_Resources)
                    //Меню выбора турелей:
                    if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_CANNON]) & 0x8000) == 0x8000)
                    {
                        //"C"annon - Турель
                        g_IFaceList->BeginBuildTurret(1);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_GUN]) & 0x8000) == 0x8000)
                    {
                        //"G"un - Пушка
                        g_IFaceList->BeginBuildTurret(2);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_LASER]) & 0x8000) == 0x8000)
                    {
                        //"L"azer - Лазер
                        g_IFaceList->BeginBuildTurret(3);
                    }
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_TURRET_ROCKET]) & 0x8000) == 0x8000)
                    {
                        //"R"ocket - Ракетница
                        g_IFaceList->BeginBuildTurret(4);
                    }
                    //Нафиг не нужно, поскольку и так сбрасывается на X
                    /*
                    else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_BUILD_TURRET]) & 0x8000) == 0x8000)
                    {
                        //"T"urret - выход из меню выбора турели
                        g_IFaceList->ResetOrderingMode();
                    }
                    */
                }
                if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_CANCEL]) & 0x8000) == 0x8000)
                {
                    //Отмена постройки турели, закрытие меню выбора (в том числе, если уже была выбрана турель)
                    if(ps->m_CurrentAction == BUILDING_TURRET)
                    {
                        ps->m_CannonForBuild.Delete();
                        ps->m_CurrentAction = NOTHING_SPECIAL;
                    }
                    g_IFaceList->ResetOrderingMode();
                }
            }
            //Общее для стратегического режима
            if((GetAsyncKeyState(g_Config.m_KeyActions[KA_MINIMAP_ZOOMIN]) & 0x8000) == 0x8000)
            {
                //приблизить карту
                g_MatrixMap->m_Minimap.ButtonZoomIn(NULL);
            }
            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_MINIMAP_ZOOMOUT]) & 0x8000) == 0x8000)
            {
                //отдалить карту
                g_MatrixMap->m_Minimap.ButtonZoomOut(NULL);
            }
            else if((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_ROBOT_SWITCH1]) & 0x8000) == 0x8000)
            {
                //","
                CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
                if(ps->GetCurGroup() && ps->GetCurGroup()->GetObjectsCnt() == 1 && ps->GetCurGroup()->m_FirstObject->GetObject()->IsLiveRobot())
                {
                    obj = ps->GetCurGroup()->m_FirstObject->GetObject()->GetPrevLogic();
                }
                int cnt = 0;
                while(1)
                {
                    if(obj)
                    {
                        if(obj->IsLiveRobot() && obj->GetSide() == PLAYER_SIDE)
                        {
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->CreateGroupFromCurrent(obj);
                            ps->Select(ROBOT, obj);
                            g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(obj->GetGeoCenter().x, obj->GetGeoCenter().y));
                            return;
                        }
                        obj = obj->GetPrevLogic();
                    }
                    else
                    {
                        if(cnt > 0)
                        {
                            return;
                        }
                        ++cnt;
                        obj = CMatrixMapStatic::GetLastLogic();
                    }
                }
            }
            else if(((GetAsyncKeyState(g_Config.m_KeyActions[KA_ORDER_ROBOT_SWITCH2]) & 0x8000) == 0x8000))
            {
                //"." 
                CMatrixMapStatic* obj = CMatrixMapStatic::GetFirstLogic();
                if(ps->GetCurGroup() && ps->GetCurGroup()->GetObjectsCnt() == 1 && ps->GetCurGroup()->m_FirstObject->GetObject()->IsLiveRobot())
                {
                    obj = ps->GetCurGroup()->m_FirstObject->GetObject()->GetNextLogic();
                }

                int cnt = 0;
                while(1)
                {
                    if(obj)
                    {
                        if(obj->IsLiveRobot() && obj->GetSide() == PLAYER_SIDE)
                        {
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->CreateGroupFromCurrent(obj);
                            ps->Select(ROBOT, obj);
                            g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(obj->GetGeoCenter().x, obj->GetGeoCenter().y));
                            return;
                        }
                        obj = obj->GetNextLogic();
                    }
                    else
                    {
                        if(cnt > 0)
                        {
                            return;
                        }
                        ++cnt;
                        obj = CMatrixMapStatic::GetFirstLogic();
                    }
                }
            }
        }

        //Если были нажаты клавиши цифр от 1 до 9 (в номенклатуре номеров это 2-10)
        if(scan > 1 && scan < 11 && !ps->IsArcadeMode() && !ps->m_ConstructPanel->IsActive())
        {
            if(CMultiSelection::m_GameSelection)
            {
                CMultiSelection::m_GameSelection->End(false);
            }

            if(ps->m_CurrentAction == BUILDING_TURRET)
            {
                ps->m_CannonForBuild.Delete();
                ps->m_CurrentAction = NOTHING_SPECIAL;
            }
            
            g_IFaceList->ResetOrderingMode();

            //Если также игрок зажал Ctrl
            if(fCtrl)
            {
                CMatrixMapStatic* o = CMatrixMapStatic::GetFirstLogic();

                //Если была выбрана группа роботов (либо один), то здесь удаляется, возможно, уже имевшаяся Ctrl-группа с нажатым номером
                while(o)
                {
                    if(o->IsRobot() && ((CMatrixRobotAI*)o)->GetCtrlGroup() == scan)
                    {
                        ((CMatrixRobotAI*)o)->SetCtrlGroup(0);
                    }
                    else if(o->GetObjectType() == OBJECT_TYPE_BUILDING && ((CMatrixBuilding*)o)->GetCtrlGroup() == scan)
                    {
                        ((CMatrixBuilding*)o)->SetCtrlGroup(0);
                    }
                    else if(o->GetObjectType() == OBJECT_TYPE_FLYER && ((CMatrixFlyer*)o)->GetCtrlGroup() == scan)
                    {
                        ((CMatrixFlyer*)o)->SetCtrlGroup(0);
                    }
                    o = o->GetNextLogic();
                }

                //Если игроком была выделена группа юнитов (либо только один), то здесь из них создаётся Ctrl-группа
                if(ps->GetCurGroup())
                {
                    CMatrixGroupObject* go = ps->GetCurGroup()->m_FirstObject;

                    while(go)
                    {
                        if(go->GetObject()->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                        {
                            ((CMatrixRobotAI*)go->GetObject())->SetCtrlGroup(scan);
                        }
                        //В формирование группы были также добавлены вертолёты (отголоски прошлого)
                        else if(go->GetObject()->GetObjectType() == OBJECT_TYPE_FLYER)
                        {
                            ((CMatrixFlyer*)go->GetObject())->SetCtrlGroup(scan);
                        }
                        go = go->m_NextObject;
                    }
                }
                //Записываем в Ctrl-группы также и выделенные строения (не турели)
                else if(ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED)
                {
                    CMatrixBuilding* go = (CMatrixBuilding*)ps->m_ActiveObject;
                    go->SetCtrlGroup(scan);
                }
            }
            //Если Ctrl игроком зажат не был
            else
            {
                //Маркер о возможной выбранности группы юнитов
                bool prev_unselected = false;

                //Если игрок повторно выбрал цифрой уже выделенную Ctrl-группу, то центрируем камеру на данную Ctrl-группу (на первого юнита в группе)
                if(ps->GetCurGroup())
                {
                    if(m_LastScans[MAX_SCANS - 1].scan == scan && m_LastScans[MAX_SCANS - 2].scan == scan && (m_LastScans[MAX_SCANS - 1].time - m_LastScans[MAX_SCANS - 2].time) < DOUBLESCAN_TIME_DELTA)
                    {
                        CMatrixMapStatic* object = NULL;
                        if(ps->GetCurGroup()->m_FirstObject) object = ps->GetCurGroup()->m_FirstObject->GetObject();

                        if(object && object->GetObjectType() == OBJECT_TYPE_ROBOTAI)
                        {
                            if(((CMatrixRobotAI*)object)->GetCtrlGroup() == scan)
                            {
                                //set camera to group position. out
                                g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(object->GetGeoCenter().x + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 0), object->GetGeoCenter().y + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 1)));
                                return;
                            }
                        }
                        else if(object && object->GetObjectType() == OBJECT_TYPE_FLYER)
                        {
                            if(((CMatrixFlyer*)object)->GetCtrlGroup() == scan)
                            {
                                //set camera to group position. out
                                g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(object->GetGeoCenter().x + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 0), object->GetGeoCenter().y + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 1)));
                                return;
                            }
                        }
                    }
                }
                //Аналогично центрируем камеру на строение из Ctrl-группы
                else
                {
                    //Устанавливаем маркер о том, что в данный момент игроком не выбрана никакая группа юнитов
                    prev_unselected = true;

                    if(ps->m_CurrSel == BASE_SELECTED || ps->m_CurrSel == BUILDING_SELECTED)
                    {
                        if(m_LastScans[MAX_SCANS - 1].scan == scan && m_LastScans[MAX_SCANS - 2].scan == scan && (m_LastScans[MAX_SCANS - 1].time - m_LastScans[MAX_SCANS - 2].time) < DOUBLESCAN_TIME_DELTA)
                        {
                            CMatrixBuilding* object = (CMatrixBuilding*)ps->m_ActiveObject;

                            if(((CMatrixBuilding*)object)->GetCtrlGroup() == scan)
                            {
                                //set camera to building position. out
                                g_MatrixMap->m_Camera.SetXYStrategy(D3DXVECTOR2(object->GetGeoCenter().x + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 0), object->GetGeoCenter().y + g_MatrixMap->m_Camera.CamAngleToCoordOffset(100, 1)));
                                return;
                            }
                        }
                    }
                }

                //Здесь происходит выделение любой заданной ранее (но в данный момент не выделенной) Ctrl-группы
                CMatrixMapStatic* o = CMatrixMapStatic::GetFirstLogic();
                while(o)
                {
                    if(o->GetSide() == PLAYER_SIDE)
                    {
                        //Возвращаем (выбираем) группу с роботом-ами
                        if(o->IsLiveRobot() && o->AsRobot()->GetCtrlGroup() == scan)
                        {
                            if(!prev_unselected)
                            {
                                prev_unselected = true;
                                ps->SelectedGroupUnselect();
                                ps->GetCurSelGroup()->RemoveAll();
                            }
                            ps->GetCurSelGroup()->AddObject(o, -4);
                        }
                        //Возвращаем (выбираем) группу со зданием
                        else if(o->GetObjectType() == OBJECT_TYPE_BUILDING && o->AsBuilding()->GetCtrlGroup() == scan)
                        {
                            //С селектами и анселектами строений тут беда, поэтому постоянно нужно "прожимать" миллион функций
                            if(!prev_unselected) prev_unselected = true;
                            ps->Select(BUILDING, o);
                            ps->GroupsUnselectSoft();
                            ps->GetCurSelGroup()->RemoveAll();
                            ps->SetCurGroup(NULL);
                            ps->Reselect();
                        }
                    }
                    o = o->GetNextLogic();
                }

                if(ps->GetCurSelGroup()->GetFlyersCnt() > 1 || ps->GetCurSelGroup()->GetRobotsCnt() > 1 || (ps->GetCurSelGroup()->GetFlyersCnt() + ps->GetCurSelGroup()->GetRobotsCnt()) > 1)
                {
                    ps->CreateGroupFromCurrent();
                    if(ps->GetCurGroup() && ps->GetCurGroup()->GetRobotsCnt() && ps->GetCurGroup()->GetFlyersCnt())
                    {
                        ps->GetCurGroup()->SortFlyers();
                    }
                    ps->Select(GROUP, NULL);
                }
                else if(ps->GetCurSelGroup()->GetFlyersCnt() == 1 && !ps->GetCurSelGroup()->GetRobotsCnt())
                {
                    ps->CreateGroupFromCurrent();
                    ps->Select(FLYER, NULL);
                }
                else if(ps->GetCurSelGroup()->GetRobotsCnt() == 1 && !ps->GetCurSelGroup()->GetFlyersCnt())
                {
                    ps->CreateGroupFromCurrent();
                    ps->Select(ROBOT, NULL);
                }
            }
        } 
//BUTTON UNPRESSED (обработчики по отжатию указанной кнопки на клавиатуре)
    if(!down)
    {
        if(scan == KEY_LSHIFT)
        {
        }

#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        if(scan == KEY_SPACE)
        {
            if(sel)
            {
                sel->Kill();
            }
            sel = 0;

            if (point.effect)
            {
                ((CMatrixEffectPointLight *)point.effect)->Kill(1000);
                point.Unconnect();
            }
            if (repair)
            {
                g_MatrixMap->SubEffect(DEBUG_CALL_INFO, repair);
                repair = 0;

            }
        }
#endif
    }
    
//BUTTON PRESSED
#if (defined _DEBUG) &&  !(defined _RELDEBUG)

        if(scan == KEY_Q)
        {
            g_MatrixMap->DumpLogic();
            g_MatrixMap->m_DI.T(L"LogicDump",L"LogicDump", 1000);
        }
        else if(/*scan == KEY_T*/0)
        {
            SSpecialBot sb;
            ZeroMemory(&sb, sizeof(SSpecialBot));

            sb.m_Armor.m_Unit.m_nKind = RUK_ARMOR_NUCLEAR;
            sb.m_Chassis.m_nKind = RUK_CHASSIS_HOVERCRAFT;
            sb.m_Weapon[0].m_Unit.m_nKind = RUK_WEAPON_MISSILE;
            sb.m_Weapon[1].m_Unit.m_nKind = RUK_WEAPON_MISSILE;
            sb.m_Weapon[2].m_Unit.m_nKind = RUK_WEAPON_LASER;
            sb.m_Weapon[3].m_Unit.m_nKind = RUK_WEAPON_PLASMA;
//            sb.m_Weapon[4].m_Unit.m_nKind = RUK_WEAPON_PLASMA;
            sb.m_Weapon[4].m_Unit.m_nKind = RUK_WEAPON_MORTAR;
            sb.m_Head.m_nKind = RUK_HEAD_BLOCKER;

            int side_id = PLAYER_SIDE;
            CMatrixSideUnit* side = g_MatrixMap->GetSideById(side_id);

            if(side->GetRobotsCnt()+side->GetRobotsInStack() >= side->GetMaxSideRobots()){
                return;
            }

            sb.m_Team = 0;
            int minr = side->m_Team[sb.m_Team].m_RobotCnt;
            for(int i = 1; i < side->m_TeamCnt; ++i)
            {
                if(side->m_Team[i].m_RobotCnt < minr)
                {
                    minr = side->m_Team[i].m_RobotCnt;
                    sb.m_Team = i;
                }
            }

            int cnt=side->GetRobotsCnt();
            CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
            while(mps)
            {
                if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                {
                    cnt+=mps->AsBuilding()->m_BS.GetItemsCnt();
                }
                mps = mps->GetNextLogic();
            }

            if(cnt<side->GetMaxSideRobots())
            {
                CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
                while(mps)
                {
                    if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                    {

                        if(mps->AsBuilding()->m_BS.GetItemsCnt()<6)
                        {
                            side->m_Constructor->SetBase(mps->AsBuilding());
                            side->m_Constructor->BuildSpecialBot(sb);
                            break;
                        }
                    }
                    mps = mps->GetNextLogic();
                }
            }
        }
        else if(/*scan == KEY_Y*/0)
        {
            SSpecialBot sb;
            ZeroMemory(&sb, sizeof(SSpecialBot));

            sb.m_Armor.m_Unit.m_nKind = RUK_ARMOR_NUCLEAR;
            sb.m_Chassis.m_nKind = RUK_CHASSIS_HOVERCRAFT;
            sb.m_Weapon[0].m_Unit.m_nKind = RUK_WEAPON_MISSILE;
            sb.m_Weapon[1].m_Unit.m_nKind = RUK_WEAPON_MISSILE;
            sb.m_Weapon[2].m_Unit.m_nKind = RUK_WEAPON_LASER;
            sb.m_Weapon[3].m_Unit.m_nKind = RUK_WEAPON_PLASMA;
//            sb.m_Weapon[4].m_Unit.m_nKind = RUK_WEAPON_PLASMA;
            sb.m_Weapon[4].m_Unit.m_nKind = RUK_WEAPON_MORTAR;
            sb.m_Head.m_nKind = RUK_HEAD_BLOCKER;

            int side_id = 2/*PLAYER_SIDE*/;
            CMatrixSideUnit* side = g_MatrixMap->GetSideById(side_id);

            if(side->GetRobotsCnt()+side->GetRobotsInStack() >= side->GetMaxSideRobots())
            {
                return;
            }

            sb.m_Team=0;
            int minr=side->m_Team[sb.m_Team].m_RobotCnt;
            for(int i = 1; i < side->m_TeamCnt; ++i)
            {
                if(side->m_Team[i].m_RobotCnt<minr)
                {
                    minr=side->m_Team[i].m_RobotCnt;
                    sb.m_Team=i;
                }
            }

            int cnt=side->GetRobotsCnt();
            CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
            while(mps)
            {
                if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                {
                    cnt+=mps->AsBuilding()->m_BS.GetItemsCnt();
                }
                mps = mps->GetNextLogic();
            }

            if(cnt<side->GetMaxSideRobots())
            {
                CMatrixMapStatic* mps = CMatrixMapStatic::GetFirstLogic();
                while(mps)
                {
                    if(mps->GetSide() == side_id && mps->GetObjectType() == OBJECT_TYPE_BUILDING && mps->AsBuilding()->IsBase())
                    {
                    
                        if(mps->AsBuilding()->m_BS.GetItemsCnt()<6)
                        {
                            side->m_Constructor->SetBase(mps->AsBuilding());
                            side->m_Constructor->BuildSpecialBot(sb);
                            break;
                        }
                    }
                    mps = mps->GetNextLogic();
                }
            }
        }

        if(scan > 1 && scan < 12)
        {}

        //if(scan==KEY_R) {
        //    CMatrixSideUnit* s = g_MatrixMap->GetPlayerSide();
        //    if(s){
        //        s->AddTitan(1000);
        //        s->AddElectronics(1000);
        //        s->AddEnergy(1000);
        //        s->AddPlasma(1000);
        //    }
        //}
        //if(scan == KEY_RBRACKET){
        //    if(g_IFaceList){
        //        g_IFaceList->SlideFocusedInterfaceRight();
        //    }
        //}

        //if(scan == KEY_LBRACKET){
        //    if(g_IFaceList){
        //        g_IFaceList->SlideFocusedInterfaceLeft();
        //    }
        //}

        if (scan == KEY_F11)
        {
            D3DResource::Dump(D3DRESTYPE_VB);
            //CCache::Dump();
        }
        if (scan == KEY_DELETE)
        {
            if (path) path->Kill();
            path = 0;
        }
        if (scan == KEY_E)
        {
            g_MatrixMap->m_Minimap.AddEvent(g_MatrixMap->m_TraceStopPos.x, g_MatrixMap->m_TraceStopPos.y, 0xFF00FF00, 0xFF000000);

        }
        if (scan == KEY_F)
        {
            g_MatrixMap->ResetMaintenanceTime();
            if (IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj) && g_MatrixMap->m_TraceStopObj->IsBuilding())
            {
                g_MatrixMap->m_TraceStopObj->AsBuilding()->Maintenance();
            }
        }
#endif
#if (defined _DEBUG) &&  !(defined _RELDEBUG)
        if(scan == KEY_F3)
        {

            static bool prev = false;
            static D3DXVECTOR3 prevp;
            D3DXVECTOR3 newp, p1, p2;

            CHelper::DestroyByGroup(9090);
            if(prev)
            {
                newp = g_MatrixMap->m_TraceStopPos;
                CHelper::Create(10000,9090)->Cone(prevp, prevp + D3DXVECTOR3(0,0,100), 30,10,0xFFFFFFFF,0xFF00FF00, 10);

                p1 = prevp;
                p2 = newp;
                prevp = newp;

            }
            else
            {
                prevp = g_MatrixMap->m_TraceStopPos;
                prev = true;
                CHelper::Create(10000,9090)->Cone(prevp, prevp + D3DXVECTOR3(0,0,100), 30,10,0xFFFFFFFF,0xFF00FF00, 10);
                return;
            }

            D3DXVECTOR3 dir(p2-p1), side;
            float len = D3DXVec3Length(&dir);
            float dd = len / 3;
            dir *= 1.0f / len;
            D3DXVec3Cross(&side, &dir, &D3DXVECTOR3(0,0,1));
            D3DXVec3Normalize(&side, &side);

            D3DXVECTOR3 to[128];
            int curi_t = 0;

            for(int idx = 1; idx < 3; ++idx)
            {
                D3DXVECTOR3 p = p1 + dir * float(idx * dd) + side * FSRND(len) * 0.2f;
                to[curi_t++] = p;

                CHelper::Create(10000,9090)->Cone(p, p + D3DXVECTOR3(0,0,90), 30,35,0xFFFFFFFF,0xFFFF0000, 10);
            }

            to[curi_t++] = p2;
            CHelper::Create(10000,9090)->Cone(p2, p2 + D3DXVECTOR3(0,0,90), 30,35,0xFFFFFFFF,0xFFFF0000, 10);
        }
        {
            //static int from = 0;
            //static int to = 100000;
            //static D3DXVECTOR3 ptfrom;

            //bool fShift=(GetAsyncKeyState(VK_SHIFT) & 0x8000)==0x8000;

            if (scan == KEY_F7)
            {
                //ptfrom = g_MatrixMap->m_Camera.GetFrustumCenter();
                //g_MatrixMap->CalcVisTemp(from, to, ptfrom);
                g_MatrixMap->CalcVis();

            }
            //if (scan == KEY_F8)
            //{
            //    if (fShift) --to; else ++to;
            //    if (to < from) to = from;

            //    CDText::T("from", CStr(from));
            //    CDText::T("to", CStr(to));

            //    g_MatrixMap->CalcVisTemp(from, to, ptfrom);
            //}
            //if (scan == KEY_F6)
            //{
            //    if (fShift) --from; else ++from;
            //    if (from > to) from = to;
            //    if (from < 0) from = 0;

            //    CDText::T("from", CStr(from));
            //    CDText::T("to", CStr(to));

            //    g_MatrixMap->CalcVisTemp(from, to, ptfrom);
            //}
        }
        //if (scan == KEY_F8)
        //{
        //    D3DXVECTOR3 p;
        //    g_MatrixMap->TraceLand(&p, g_MatrixMap->m_Camera.GetFrustumCenter(), g_MatrixMap->m_MouseDir);
        //    CHelper::Create(10000,888)->Line(g_MatrixMap->m_Camera.GetFrustumCenter(), p);
        //}
        if (scan == KEY_F6)
        {
            const D3DXVECTOR3 &cam = g_MatrixMap->m_Camera.GetFrustumCenter();
            int gx = TruncFloat(cam.x / (GLOBAL_SCALE * MAP_GROUP_SIZE));
            int gy = TruncFloat(cam.y / (GLOBAL_SCALE * MAP_GROUP_SIZE));

            if (gx >= 0 && gx < g_MatrixMap->m_GroupSize.x && gy >= 0 && gy < g_MatrixMap->m_GroupSize.y)
            {
                SGroupVisibility *gv = g_MatrixMap->m_GroupVis + gx + gy * g_MatrixMap->m_GroupSize.x;

                int level = TruncFloat((cam.z - gv->z_from) / GLOBAL_SCALE);

                if (level >=0 && level < gv->levels_cnt)
                {
                    int n = gv->levels[level];

                    CHelper::DestroyByGroup(345);
                    for (int i = 0; i < n; ++i)
                    {
                        PCMatrixMapGroup g = gv->vis[i];

                        D3DXVECTOR3 f((g->GetPos0().x + g->GetPos1().x) * 0.5f, (g->GetPos0().y + g->GetPos1().y) * 0.5f, 0);
                        f.z = g_MatrixMap->GetZ(f.x, f.y);

                        CHelper::Create(10000,345)->Cone(f, f + D3DXVECTOR3(0,0,100), 30,30,0xFF00FF00, 0xFF0000FF,20);
                        
                    }

                }
            }
        }

        if (scan == KEY_F5 && IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj))
        {
            if (g_MatrixMap->m_TraceStopObj->IsRobot())
            {
                g_MatrixMap->m_TraceStopObj->AsRobot()->m_PosX = 4110.0f;
                g_MatrixMap->m_TraceStopObj->AsRobot()->m_PosY = 2295.0f;
                g_MatrixMap->m_TraceStopObj->RChange(MR_Matrix);
                g_MatrixMap->m_TraceStopObj->RNeed(MR_Matrix);
                g_MatrixMap->m_TraceStopObj->JoinToGroup();


                CPoint sss(Float2Int(4110.0f / GLOBAL_SCALE_MOVE), Float2Int(2295.0f / GLOBAL_SCALE_MOVE));

                g_MatrixMap->GetPlayerSide()->PGOrderAttack(g_MatrixMap->GetPlayerSide()->RobotToLogicGroup(g_MatrixMap->m_TraceStopObj->AsRobot()),sss,NULL);

            }

        }
        if (scan == KEY_K && IS_TRACE_STOP_OBJECT(g_MatrixMap->m_TraceStopObj))
        {
            CMatrixMapStatic *f = g_MatrixMap->m_TraceStopObj;
            if (f->GetObjectType() == OBJECT_TYPE_FLYER || f->GetObjectType() == OBJECT_TYPE_ROBOTAI)
            {
                CMatrixMapStatic::SortBegin();
                
                if (f->IsFlyer())
                    ((CMatrixFlyer *)f)->SetHitpoint(1);
                f->Damage(WEAPON_FLAMETHROWER,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);
            }
            else if (f->GetObjectType() == OBJECT_TYPE_MAPOBJECT)
            {
                f->Damage(WEAPON_BOMB,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);

            }
            else if (f->GetObjectType() == OBJECT_TYPE_BUILDING)
            {
                bool killed = f->Damage(WEAPON_BIGBOOM,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);
                if (!killed) killed = f->Damage(WEAPON_BIGBOOM,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);
                if (!killed) killed = f->Damage(WEAPON_BIGBOOM,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);
                if (!killed) killed = f->Damage(WEAPON_BIGBOOM,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);
                if (!killed) killed = f->Damage(WEAPON_BIGBOOM,f->GetGeoCenter(), D3DXVECTOR3(0,0,0),0,NULL);
            }

        }
        if (scan == KEY_SPACE)
        {
            D3DXVECTOR3 pos = g_MatrixMap->m_TraceStopPos;
            pos.z += 10.0f;

            if (0)
            if (point.effect == NULL)
            {
                CMatrixEffect::CreatePointLight(&point, pos + D3DXVECTOR3(0,0,0), 150, 0xFF0033FF, false);
            }
            
            if (0)
            if (!sel)
            {
                sel = (CMatrixEffectSelection *)CMatrixEffect::CreateSelection(pos, 20);
                g_MatrixMap->AddEffect(sel);
            }
            if (0)
            if (!repair)
            {
                repair = (CMatrixEffectRepair *)CMatrixEffect::CreateRepair(pos, D3DXVECTOR3(1,0,0), 200, NULL);
                g_MatrixMap->AddEffect(repair);
            }
            
		}
#endif
            

    }
}

void CFormMatrixGame::SystemEvent(ESysEvent se)
{
    DTRACE();
    if (se == SYSEV_DEACTIVATING)
    {
        if (FLAG(g_MatrixMap->m_Flags, MMFLAG_VIDEO_RESOURCES_READY))
        {
            g_MatrixMap->ReleasePoolDefaultResources();
        }
    }

    if (se == SYSEV_ACTIVATED)
    {
        if (FLAG(g_Flags, GFLAG_FULLSCREEN))
        {

            RECT r;
            GetWindowRect(g_Wnd, &r);
            ClipCursor( &r );
        }
    }
}
