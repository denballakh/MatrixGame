// MatrixGame - SR2 Planetary battles engine
// Copyright (C) 2012, Elemental Games, Katauri Interactive, CHK-Games
// Licensed under GPLv2 or any later version
// Refer to the LICENSE file included

// MatrixGame.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "MatrixGame.h"
#include "MatrixMap.hpp"
#include "MatrixFormGame.hpp"
#include "Interface/CInterface.h"
#include "MatrixRenderPipeline.hpp"
#include "MatrixGameDll.hpp"
#include "ShadowStencil.hpp"
#include "MatrixLoadProgress.hpp"
#include "MatrixFlyer.hpp"
#include "MatrixMapStatic.hpp"
#include "MatrixMultiSelection.hpp"
#include "MatrixTerSurface.hpp"
#include "MatrixSkinManager.hpp"
#include "MatrixSoundManager.hpp"
#include "Interface/CIFaceMenu.h"
#include "Interface/MatrixHint.hpp"
#include "Interface/CHistory.h"
#include "MatrixInstantDraw.hpp"
#include "MatrixSampleStateManager.hpp"

#include <stdio.h>
#include <time.h>

#include <ddraw.h>

////////////////////////////////////////////////////////////////////////////////
CHeap* g_MatrixHeap;
CBlockPar* g_MatrixData;
CMatrixMapLogic* g_MatrixMap;
CRenderPipeline* g_Render;
CLoadProgress* g_LoadProgress;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE ,
                     LPTSTR    ,
                     int       )
{


    //CBitmap b,bb;
    //b.LoadFromPNG(L"test.png");
    //bb.CreateRGB(b.SizeX(), b.SizeY());
    //bb.Copy(CPoint(0,0), b.Size(), b, CPoint(0,0));
    //bb.SaveInDDSUncompressed(L"out.dds");

    //CBuf bla;
    //bla.LoadFromFile(L"test.dds");
    //BYTE *data = (BYTE *)bla.Get();
    //DDSURFACEDESC2 *desc = (DDSURFACEDESC2 *)(data + 4);

    //int x = FP_NORM_TO_BYTE2(1.0f);
    //x = FP_NORM_TO_BYTE2(0.9999f);
    //x = FP_NORM_TO_BYTE2(0.5f);
    //x = FP_NORM_TO_BYTE2(0.1f);
    //x = FP_NORM_TO_BYTE2(0.0f);
    //x = FP_NORM_TO_BYTE2(2.0f);
    //x = FP_NORM_TO_BYTE2(200.6f);
    //x = FP_NORM_TO_BYTE2(255.6f);
    //x = FP_NORM_TO_BYTE2(256.6f);


    //float t,k;
    //k = 10.0f; FP_INV(t, k);
    //k = 20.0f; FP_INV(t, k);
    //k = 25.0f; FP_INV(t, k);

    const wchar *cmd = GetCommandLineW();

    int numarg;
    wchar** args = CommandLineToArgvW(cmd, &numarg);
    wchar* map = NULL;

    if(numarg > 1)
    {
        map = args[1];

    }

	try
    {
		srand((unsigned)time( NULL ));

		MatrixGameInit(hInstance, NULL, map);

		CFormMatrixGame * formgame=HNew(NULL) CFormMatrixGame();
		FormChange(formgame);

		timeBeginPeriod(1);

//DWORD * buf=(DWORD *)HAlloc(124,NULL);
//*(buf-1)=1;
//HFree(buf,NULL);

        if(map)
        {
            FILE *file;
            file = fopen("calcvis.log","a");
            CStr name(g_MatrixMap->MapName(), g_MatrixHeap);
            fwrite(name.Get(), name.Len(),1, file);
            fwrite(" ...", 4,1, file);
            fclose(file);

            g_MatrixMap->CalcVis();

            file = fopen("calcvis.log","a");
            fwrite("done\n", 5,1, file);
            fclose(file);
        }
        else
        {
		    L3GRun();
        }

		timeEndPeriod(1);

		MatrixGameDeinit();

        FormChange(NULL);
        HDelete(CFormMatrixGame, formgame, NULL);


        g_Cache->Clear();
        L3GDeinit();
		CacheDeinit();

        CMain::BaseDeInit();
        
    }
    catch(CException * ex)
    {
        ClipCursor(NULL);
#ifdef ENABLE_HISTORY
        CDebugTracer::SaveHistory();
#endif
        g_Cache->Clear();
		L3GDeinit();
        
		MessageBox(NULL,CStr(ex->Info()).Get(),"Exception:",MB_OK);

		delete ex;
	}
    catch(...)
    {
#ifdef ENABLE_HISTORY
        CDebugTracer::SaveHistory();
#endif
        MessageBox(NULL,"Unknown bug :(","Exception:",MB_OK);

    }
    
    ClipCursor(NULL);

	return 1;
}

static void static_init(void)
{

    // Base
    CMain::BaseInit();

    // 3G
#ifdef _DEBUG
    D3DResource::StaticInit();
#endif
#if(defined _DEBUG) && !(defined _RELDEBUG)
    CHelper::StaticInit();
#endif
    CCacheData::StaticInit();
    CVectorObject::StaticInit();
    CVOShadowProj::StaticInit();
    CVOShadowStencil::StaticInit();
    CBillboard::StaticInit();
    CBillboardLine::StaticInit();
    CForm::StaticInit();


    // Game
    CMatrixProgressBar::StaticInit();
    CMatrixMapStatic::StaticInit();
    CMatrixMapObject::StaticInit();
    CMatrixFlyer::StaticInit();
    CMatrixMapGroup::StaticInit();
    CInterface::StaticInit();
    CMultiSelection::StaticInit();
    CTerSurface::StaticInit();
    CBottomTextureUnion::StaticInit();
    CSkinManager::StaticInit();
    CMatrixHint::StaticInit();
    CInstDraw::StaticInit();
    SInshorewave::StaticInit();

    g_Flags = 0; //GFLAG_FORMACCESS;
}

void LoadModList(CWStr& modlist)
{
    DTRACE();

    CWStr modcfg_name(g_MatrixHeap);
    CBlockPar modcfg(g_MatrixHeap);
    
    if(CFile::FileExist(modcfg_name, L"Mods\\ModCFG.txt"))
    {
        modcfg.LoadFromTextFile(L"Mods\\ModCFG.txt");
        modlist.Set(modcfg.ParGet(L"CurrentMod"));
    }
    else modlist.Set(L"");
}

void LoadCfgFromMods(CWStr &modlist, CBlockPar &base_bp, const wchar *lang, const wchar *bp_name)
{
    DTRACE();

    CWStr curmod(g_MatrixHeap);
    CWStr moddata_name(g_MatrixHeap);

    CBlockPar moddata(g_MatrixHeap);

    for(int i = -1; i < modlist.GetCountPar(L","); ++i)
    {
        //Мод PBEngine заносится в список вне очереди
        if(i < 0) curmod = L"PlanetaryBattles\\PBEngine";
        else
        {
            curmod = modlist.GetStrPar(i, L",");
            curmod.Trim();

            if(curmod == L"PlanetaryBattles\\PBEngine") continue;
        }

        moddata_name.Clear();
        moddata_name += L"Mods\\";
        moddata_name += curmod;
        moddata_name += L"\\";
        moddata_name += FILE_CONFIGURATION_LOCATION;

        if(lang != NULL)
        {
            moddata_name += lang;
            moddata_name += L"\\";
        }
        moddata_name += L"Robots\\";
        moddata_name += bp_name;

        if(CFile::FileExist(moddata_name, moddata_name.Get()))
        {
            moddata.LoadFromTextFile(moddata_name);
            base_bp.BlockMerge(moddata);
        }
    }
}

void MatrixGameInit(
    HINSTANCE inst,       //???
    HWND wnd,             //???
    wchar *map,           //Предположительно, путь к файлу карты
    SRobotsSettings *set, //???
    wchar *lang,          //Язык, Rus или Eng, используется при построении путей к папкам с языковыми файлами
    wchar *txt_start,     //Вступительный текст
    wchar *txt_win,       //Текст при победе
    wchar *txt_loss,      //Текст при поражении
    wchar *planet         //Название планеты
)
{
    static_init();

    DTRACE();

    g_MatrixHeap = HNew(NULL) CHeap;

    CFile::AddPackFile(L"DATA\\robots.pkg", NULL);
    CFile::OpenPackFiles();

    CWStr modlist(g_MatrixHeap);
    LoadModList(modlist);

    CLoadProgress     lp;
    g_LoadProgress = &lp;
DCP();

    CStorage    stor_cfg(g_MatrixHeap);
    bool        stor_cfg_present = false;
    CWStr       stor_cfg_name(g_MatrixHeap);
	wchar       conf_file[80];
	wcscpy(conf_file, FILE_CONFIGURATION_LOCATION);

	if(lang != NULL)
    {
	    wcscat(conf_file, lang);
	    wcscat(conf_file, L"\\");
    }

	wcscat(conf_file, FILE_CONFIGURATION);

    if(CFile::FileExist(stor_cfg_name, conf_file))
    {
        stor_cfg.Load(conf_file);
        stor_cfg_present = true;
    }

	g_MatrixData = HNew(g_MatrixHeap) CBlockPar(1, g_MatrixHeap);
    if(stor_cfg_present)
    {
        stor_cfg.RestoreBlockPar(L"da", *g_MatrixData);
		//stor_cfg.RestoreBlockPar(L"if", *g_MatrixData);
        //g_MatrixData->SaveInTextFile(L"bbb.txt");

        if(CFile::FileExist(stor_cfg_name, L"cfg\\robots\\cfg.txt"))
        {
            CBlockPar* bpc = g_MatrixData->BlockGet(L"Config");
            bpc->LoadFromTextFile(L"cfg\\robots\\cfg.txt");
        }
    }
    else
    {
        g_MatrixData->LoadFromTextFile(L"cfg\\robots\\data.txt");
    }

    // Code for loading data configs from mods
    LoadCfgFromMods(modlist, *g_MatrixData, lang, L"data.txt");

    CBlockPar *repl = g_MatrixData->BlockGetAdd(PAR_REPLACE);

    // init menu replaces

    CBlockPar *rr = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Replaces");
    int cnt = rr->ParCount();
    for(int i = 0; i < cnt; ++i)
    {
        repl->ParAdd(rr->ParGetName(i), rr->ParGet(i));
    }

    if(txt_start)
    {
        if(txt_start[0] >= '1' && txt_start[0] <= '6')
        {
            repl->ParSetAdd(PAR_REPLACE_BEGIN_ICON_RACE, CWStr(txt_start, 1, g_MatrixHeap));
            repl->ParSetAdd(PAR_REPLACE_DIFFICULTY, CWStr(txt_start + 1, 2, g_MatrixHeap));
            repl->ParSetAdd(PAR_REPLACE_BEGIN_TEXT, txt_start + 3);
        }
        else
        {
            repl->ParSetAdd(PAR_REPLACE_BEGIN_ICON_RACE, L"1");
            repl->ParSetAdd(PAR_REPLACE_BEGIN_TEXT, txt_start);
        }
    }
    else
    {
        repl->ParSetAdd(PAR_REPLACE_BEGIN_ICON_RACE, L"1");
        repl->ParSetAdd(PAR_REPLACE_BEGIN_TEXT, L"Go! Go! Go!");
    }

    if(txt_win)
    {
        repl->ParSetAdd(PAR_REPLACE_END_TEXT_WIN, txt_win);
    }
    else
    {
        repl->ParSetAdd(PAR_REPLACE_END_TEXT_WIN, L"Good job man :)");
    }

    if(txt_loss)
    {
        repl->ParSetAdd(PAR_REPLACE_END_TEXT_LOOSE, txt_loss);
    }
    else
    {
        repl->ParSetAdd(PAR_REPLACE_END_TEXT_LOOSE, L"Sux :(");
    }

    if(planet)
    {
        repl->ParSetAdd(PAR_REPLACE_END_TEXT_PLANET, planet);
    }
    else
    {
        repl->ParSetAdd(PAR_REPLACE_END_TEXT_PLANET, L"Luna");
    }

DCP();

	CacheInit();
    LoadCfgFromMods(modlist, *g_CacheData, lang, L"cache.txt");
DCP();


    /*
    if(g_RangersInterface)
    {
        // run from rangers
        // do not set fullscreen
        g_MatrixData->BlockGet(L"Config")->ParSet(L"FullScreen", L"0");
    }
    */
	
#ifdef _DEBUG
	g_MatrixData->BlockGet(L"Config")->SaveInTextFile(L"g_ConfigDump.txt");
#endif

	if(set) L3GInitAsDLL(inst, *g_MatrixData->BlockGet(L"Config"), L"MatrixGame", L"Matrix Game", wnd, set->FDirect3D, set->FD3DDevice);
	else L3GInitAsEXE(inst, *g_MatrixData->BlockGet(L"Config"), L"MatrixGame", L"Matrix Game");
	

    if(set)
    {
        g_ScreenX = set->m_ResolutionX;
        g_ScreenY = set->m_ResolutionY;
    }

    g_Render = HNew(g_MatrixHeap) CRenderPipeline;   // prepare pipelines

	ShowWindow(g_Wnd, SW_SHOWNORMAL);
	UpdateWindow(g_Wnd);

DCP();

    g_Config.SetDefaults();
    g_Config.ReadParams();
    if(set)
	{
		g_Config.ApplySettings(set);
		g_Sampler.ApplySettings(set);
	}

    CStorage stor(g_CacheHeap);
DCP();

    CWStr mapname(g_CacheHeap);

    if(map)
    {
		if(wcschr(map, '\\') == NULL)
        {
			mapname.Set(L"Matrix\\Map\\");
			mapname.Add(map);
		}
        else
        {
			mapname.Set(map);
		}
    }
    else
    {
        mapname = g_MatrixData->BlockGet(L"Config")->Par(L"Map");
    }

    stor.Load(mapname);

    // Load replacements for MatrixData from map
    if (stor.IsTablePresent(L"da"))
    {
        CBlockPar map_data(g_CacheHeap);
        stor.RestoreBlockPar(L"da", map_data);
        g_MatrixData->BlockMerge(map_data);
    }

DCP();

	g_MatrixMap =  HNew(g_MatrixHeap) CMatrixMapLogic;

	g_MatrixMap->LoadSide(*g_MatrixData->BlockGet(L"Side"));
	//g_MatrixMap->LoadTactics(*g_MatrixData->BlockGet(L"Tactics"));
	g_IFaceList = HNew(g_MatrixHeap) CIFaceList;

	// load new map
DCP();
    g_MatrixMap->RobotPreload();
    
DCP();
	
    if(0 > g_MatrixMap->PrepareMap(stor, mapname))
    {
        ERROR_S(L"Unable to load map. Error happens.");
    }

    CWStr mn(g_MatrixMap->MapName(), g_MatrixHeap);
    mn.LowerCase();
    if(mn.Find(L"demo") >= 0)
    {
        SETFLAG(g_MatrixMap->m_Flags, MMFLAG_AUTOMATIC_MODE|MMFLAG_FLYCAM|MMFLAG_FULLAUTO);
    }

    g_MatrixMap->CalcCannonPlace();
    SSpecialBot::LoadAIRobotType(*g_MatrixData->BlockGet(L"AIRobotType"));

    g_LoadProgress->SetCurLP(LP_PREPARININTERFACE);
    g_LoadProgress->InitCurLP(701);


    CBlockPar bpi(1, g_CacheHeap);
    if(stor_cfg_present)
    {
        stor_cfg.RestoreBlockPar(L"if", bpi);
    }
    else
    {
        bpi.LoadFromTextFile(IF_PATH);

    //CStorage stor(g_CacheHeap);
    //stor.StoreBlockPar(L"if", bpi);
    //stor.StoreBlockPar(L"da", *g_MatrixData);
    //stor.Save(FILE_CONFIGURATION, true);
    }

    // The code for loading iface configs from mods
    LoadCfgFromMods(modlist, bpi, lang, L"iface.txt");

#ifdef _DEBUG
    g_MatrixData->SaveInTextFile(L"g_MatrixData.txt");
#endif


    g_ConfigHistory = HNew(g_MatrixHeap) CHistory;
    CInterface *pInterface = NULL;

    g_LoadProgress->SetCurLPPos(10);

    CMatrixHint::PreloadBitmaps();

    bool iface_save = false;

    g_PopupHead = (SMenuItemText*)HAlloc(sizeof(SMenuItemText)*MENU_HEAD_ITEMS, g_MatrixHeap);
    for(int i = 0; i < MENU_HEAD_ITEMS; ++i)
    {
        g_PopupHead[i].SMenuItemText::SMenuItemText(g_MatrixHeap);
    }

    g_PopupHull = (SMenuItemText*)HAlloc(sizeof(SMenuItemText)*MENU_HULL_ITEMS, g_MatrixHeap);
    for(int i = 0; i < MENU_HULL_ITEMS; ++i)
    {
        g_PopupHull[i].SMenuItemText::SMenuItemText(g_MatrixHeap);
    }

    g_PopupWeaponNormal = (SMenuItemText*)HAlloc(sizeof(SMenuItemText)*MENU_WEAPONNORM_ITEMS, g_MatrixHeap);
    for(int i = 0; i < MENU_WEAPONNORM_ITEMS; ++i)
    {
        g_PopupWeaponNormal[i].SMenuItemText::SMenuItemText(g_MatrixHeap);
    }

    g_PopupWeaponExtern = (SMenuItemText*)HAlloc(sizeof(SMenuItemText)*MENU_WEAPONEXTERN_ITEMS, g_MatrixHeap);
    for(int i = 0; i < MENU_WEAPONEXTERN_ITEMS; ++i)
    {
        g_PopupWeaponExtern[i].SMenuItemText::SMenuItemText(g_MatrixHeap);
    }

    g_PopupChassis = (SMenuItemText*)HAlloc(sizeof(SMenuItemText)*MENU_CHASSIS_ITEMS, g_MatrixHeap);
    for(int i = 0; i < MENU_CHASSIS_ITEMS; ++i)
    {
        g_PopupChassis[i].SMenuItemText::SMenuItemText(g_MatrixHeap);
    }
    
    CIFaceMenu::m_MenuGraphics = HNew(g_MatrixHeap) CInterface;

    g_PopupMenu = HNew(g_MatrixHeap) CIFaceMenu;

    pInterface = HNew(g_MatrixHeap) CInterface;
	iface_save |= pInterface->Load(bpi, IF_TOP);
	LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(100);

    pInterface = HNew(g_MatrixHeap) CInterface;
	iface_save |= pInterface->Load(bpi, IF_MINI_MAP);
	LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(200);

    pInterface = HNew(g_MatrixHeap) CInterface;
	iface_save |= pInterface->Load(bpi, IF_RADAR);
	LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(300);

    pInterface = HNew(g_MatrixHeap) CInterface;
	iface_save |= pInterface->Load(bpi, IF_BASE);
	LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);
    
    g_IFaceList->m_BaseX = Float2Int(pInterface->m_xPos);
    g_IFaceList->m_BaseY = Float2Int(pInterface->m_yPos);
    g_LoadProgress->SetCurLPPos(400);

    pInterface = HNew(g_MatrixHeap) CInterface;
	iface_save |= pInterface->Load(bpi, IF_MAIN);
    g_IFaceList->SetMainPos(pInterface->m_xPos, pInterface->m_yPos);
	LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_LoadProgress->SetCurLPPos(500);

    pInterface = HNew(g_MatrixHeap) CInterface;
	iface_save |= pInterface->Load(bpi, IF_HINTS);
	LIST_ADD(pInterface, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);
    g_IFaceList->m_Hints = pInterface;

    g_LoadProgress->SetCurLPPos(600);

    iface_save |= CIFaceMenu::LoadMenuGraphics(bpi);
	//LIST_ADD(CIFaceMenu::m_MenuGraphics, g_IFaceList->m_First, g_IFaceList->m_Last, m_PrevInterface, m_NextInterface);

    g_IFaceList->ConstructorButtonsInit();

    g_LoadProgress->SetCurLPPos(700);

    g_PopupWeaponExtern[0].text = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Base")->ParGet(L"none");
    g_PopupWeaponNormal[0].text = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Base")->ParGet(L"none");
    g_PopupHead[0].text = g_MatrixData->BlockGet(IF_LABELS_BLOCKPAR)->BlockGet(L"Base")->ParGet(L"none");


    if(g_RangersInterface)
    {
        g_MatrixMap->m_Transition.BuildTexture();
		//g_D3DD->Reset(&g_D3Dpp);
        g_RangersInterface->m_Begin();
    }

    if(set) set->ApplyVideoParams();
	else g_D3DD->Reset(&g_D3Dpp);

	/*IDirect3DSurface9 * surf;
	g_D3DD->GetRenderTarget(0, &surf);
	if(!(surf == NULL)) g_D3DD->ColorFill(surf, NULL, 0);
	surf->Release();*/

    g_MatrixMap->m_Transition.RenderToPrimaryScreen();

    CMatrixEffect::InitEffects(*g_MatrixData);
    g_MatrixMap->CreatePoolDefaultResources(true);
    g_MatrixMap->InitObjectsLights();

    g_MatrixMap->GetPlayerSide()->Select(BUILDING, g_MatrixMap->GetPlayerSide()->m_ActiveObject);
    g_MatrixMap->m_Cursor.Select(CURSOR_ARROW);

    if(!FLAG(g_MatrixMap->m_Flags, MMFLAG_FULLAUTO)) g_MatrixMap->EnterDialogMode(TEMPLATE_DIALOG_BEGIN);

    //Проверяем, включена ли настройка сохранения шаблонов в конфиге
    int maxDesignsToSave = g_MatrixData->BlockGet(L"Config")->ParGet(CFG_DESIGNS_TO_SAVE).GetInt();

    //Загружаем сохранённые шаблоны роботов игрока из txt-конфига и заносим их в конструктор роботов
    if(maxDesignsToSave > 0)
    {
        wchar robotsCFG_path[MAX_PATH];
        SHGetSpecialFolderPathW(0, robotsCFG_path, CSIDL_PERSONAL, true);
        wcscat(robotsCFG_path, L"\\SpaceRangersHD\\RobotsCFG.txt");

        //Если обнаружили искомый конфиг
        CWStr temp_path(g_MatrixHeap);
        if(CFile::FileExist(temp_path, robotsCFG_path))
        {
            CBlockPar robotsCFG = CBlockPar(true, g_MatrixHeap);
            robotsCFG.LoadFromTextFile(robotsCFG_path);
            CBlockPar* bot_designs = robotsCFG.BlockGetNE(L"BotDesigns");
            if(bot_designs != nullptr)
            {
                int counter = 0;
                //Расшифровываем строку цифро-буквенного шаблона робота, полученную из txt-конфига
                CWStr confStr;
                SRobotConfig conf;
                while(counter < maxDesignsToSave)
                {
                    confStr = bot_designs->ParGetNE(L"Bot" + CWStr(counter + 1));
                    if(confStr.IsEmpty()) break;

                    //Исключаем заведомо невалидные по длине шаблоны
                    if(confStr.GetCountPar(L",") < 2) continue;

                    // предварительно очищаем и инициализируем шаблон
                    ZeroMemory(&conf, sizeof(conf)); 
                    conf.m_Chassis.m_nType = MRT_CHASSIS; 
                    conf.m_Hull.m_Unit.m_nType = MRT_ARMOR; 
                    conf.m_Head.m_nType = MRT_HEAD; 
                    for(int cnt = 0; cnt < MAX_WEAPON_CNT; ++cnt)
                    {
                        conf.m_Weapon[cnt].m_nType = MRT_WEAPON;
                    }

                    //Определяем шасси
                    CWStr t_str = confStr.GetStrPar(0, L",");
                    t_str.Trim();
                    if(t_str == L"P") conf.m_Chassis.m_nKind = RUK_CHASSIS_PNEUMATIC;
                    else if(t_str == L"W") conf.m_Chassis.m_nKind = RUK_CHASSIS_WHEEL;
                    else if(t_str == L"T") conf.m_Chassis.m_nKind = RUK_CHASSIS_TRACK;
                    else if(t_str == L"H") conf.m_Chassis.m_nKind = RUK_CHASSIS_HOVERCRAFT;
                    else if(t_str == L"A") conf.m_Chassis.m_nKind = RUK_CHASSIS_ANTIGRAVITY;
                    else continue;

                    //Определяем корпус
                    t_str = confStr.GetStrPar(1, L",");
                    t_str.Trim();
                    if(t_str == L"1") conf.m_Hull.m_Unit.m_nKind = RUK_ARMOR_6;
                    else if(t_str == L"1S") conf.m_Hull.m_Unit.m_nKind = RUK_ARMOR_PASSIVE;
                    else if(t_str == L"2") conf.m_Hull.m_Unit.m_nKind = RUK_ARMOR_ACTIVE;
                    else if(t_str == L"2S") conf.m_Hull.m_Unit.m_nKind = RUK_ARMOR_FIREPROOF;
                    else if(t_str == L"3") conf.m_Hull.m_Unit.m_nKind = RUK_ARMOR_PLASMIC;
                    else if(t_str == L"4S") conf.m_Hull.m_Unit.m_nKind = RUK_ARMOR_NUCLEAR;
                    else continue;

                    //Записываем в конфиг предельное число обычных и особых орудий для данного типа корпуса
                    conf.m_Hull.m_MaxCommonWeaponCnt = g_MatrixMap->m_RobotWeaponMatrix[conf.m_Hull.m_Unit.m_nKind - 1].common;
                    conf.m_Hull.m_MaxExtraWeaponCnt = g_MatrixMap->m_RobotWeaponMatrix[conf.m_Hull.m_Unit.m_nKind - 1].extra;

                    //Определяем оружие
                    if(confStr.GetCountPar(L",") >= 3)
                    {
                        t_str = confStr.GetStrPar(2, L",");
                        t_str.Trim();

                        if(t_str.GetLen() > (conf.m_Hull.m_MaxCommonWeaponCnt + conf.m_Hull.m_MaxExtraWeaponCnt)) continue;

                        int cnt_normal = 0;
                        int cnt_extra = 0;

                        for(int u = 0; u < t_str.GetLen(); ++u)
                        {
                            wchar ch = t_str.GetBuf()[u];
                            if(ch == L'G')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_MACHINEGUN;
                            }
                            else if(ch == L'C')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_CANNON;
                            }
                            else if(ch == L'M')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_MISSILE;
                            }
                            else if(ch == L'F')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_FLAMETHROWER;
                            }
                            else if(ch == L'L')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_LASER;
                            }
                            else if(ch == L'P')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_PLASMA;
                            }
                            else if(ch == L'E')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_ELECTRIC;
                            }
                            else if(ch == L'R')
                            {
                                ++cnt_normal;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_REPAIR;
                            }
                            else if(ch == L'O')
                            {
                                u = 4; // Установка оружия в слот для особого вооружения
                                ++cnt_extra;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_MORTAR;
                            }
                            else if(ch == L'B')
                            {
                                u = 4; // Установка оружия в слот для особого вооружения
                                ++cnt_extra;
                                conf.m_Weapon[u].m_nKind = RUK_WEAPON_BOMB;
                            }
                            else continue;

                            conf.m_Weapon[u].m_nType = MRT_WEAPON;
                        }

                        if(cnt_normal > conf.m_Hull.m_MaxCommonWeaponCnt) continue;

                        if(cnt_extra > conf.m_Hull.m_MaxExtraWeaponCnt) continue;
                    }

                    //Определяем голову
                    if(confStr.GetCountPar(L",") >= 4)
                    {
                        t_str = confStr.GetStrPar(3, L",");
                        t_str.Trim();

                        if(t_str == L"S") conf.m_Head.m_nKind = RUK_HEAD_BLOCKER;
                        else if(t_str == L"D") conf.m_Head.m_nKind = RUK_HEAD_DYNAMO;
                        else if(t_str == L"L") conf.m_Head.m_nKind = RUK_HEAD_LOCKATOR;
                        else if(t_str == L"F") conf.m_Head.m_nKind = RUK_HEAD_FIREWALL;
                        else if(t_str == L"R") conf.m_Head.m_nKind = RUK_HEAD_RAPID;
                        else if(t_str == L"D") conf.m_Head.m_nKind = RUK_HEAD_DESIGN;
                        else if(t_str == L"P") conf.m_Head.m_nKind = RUK_HEAD_SPEAKER;
                        else continue;
                    }

                    //Заносим готовый конфиг в историю
                    g_ConfigHistory->AddConfig(&conf);

                    ++counter;
                }
                //Убираем начальный стоковый чертёж завода и заменяем его на первый из списка загруженных
                g_ConfigHistory->m_CurrentConfig = g_ConfigHistory->m_FirstConfig;
                g_ConfigHistory->LoadCurrentConfigToConstructor();
            }
        }
    }

    // this code can be safely removed from release : RELEASE_OFF

    //if(iface_save) bpi.SaveInTextFile(IF_PATH, true);
}

void SRobotsSettings::ApplyVideoParams(void)
{
    DTRACE();
    
    int bpp;
	D3DDISPLAYMODE d3ddm;

	D3DDEVICE_CREATION_PARAMETERS params;
	g_D3DD->GetCreationParameters(  &params );
	//ASSERT_DX(g_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm));
	ASSERT_DX(g_D3D->GetAdapterDisplayMode(params.AdapterOrdinal,&d3ddm));

    if(d3ddm.Format == D3DFMT_X8R8G8B8 || d3ddm.Format == D3DFMT_A8R8G8B8)
    {
        d3ddm.Format=D3DFMT_A8R8G8B8;
        bpp = 32;
    }
    else
    {
        bpp = 16;
    }

    bool change_refresh_rate = m_RefreshRate != 0 && m_RefreshRate != d3ddm.RefreshRate;
    int  refresh_rate_required = change_refresh_rate ? m_RefreshRate : 0;

	RECT rect;
    GetClientRect(g_Wnd, &rect);
	bool was_in_window_mode = (rect.right != d3ddm.Width || rect.bottom != d3ddm.Height);
	bool now_in_window_mode = was_in_window_mode && (bpp == m_BPP) && !change_refresh_rate;
	
	if (m_FSAASamples > 16) ERROR_S(L"Invalid multisample type");
	_D3DMULTISAMPLE_TYPE expectedMultiSampleType = (_D3DMULTISAMPLE_TYPE)m_FSAASamples;//(m_FSAASamples >> 24);	
	
	ZeroMemory( &g_D3Dpp, sizeof(g_D3Dpp) );

	if(now_in_window_mode)
    {
		RESETFLAG(g_Flags, GFLAG_FULLSCREEN);
        g_D3Dpp.Windowed = TRUE;

        RECT r1,r2;
        GetWindowRect(g_Wnd, &r1);
        GetClientRect(g_Wnd, &r2);
        SetWindowPos(g_Wnd, NULL, 0,0, m_ResolutionX + (r1.right-r1.left-r2.right), m_ResolutionY+(r1.bottom-r1.top-r2.bottom), SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
	}
    else
    {
		SETFLAG(g_Flags, GFLAG_FULLSCREEN);
        g_D3Dpp.Windowed = FALSE;

        if(was_in_window_mode)
        {
            SetWindowLong(g_Wnd, GWL_STYLE, WS_POPUP|WS_VISIBLE);
            MoveWindow(g_Wnd, 0, 0, m_ResolutionX, m_ResolutionY, false);
        }
	}


		g_D3Dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        g_D3Dpp.BackBufferFormat = (m_BPP==16)?D3DFMT_R5G6B5:D3DFMT_A8R8G8B8;
        g_D3Dpp.EnableAutoDepthStencil = TRUE;
		g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		g_D3Dpp.PresentationInterval = m_VSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
        g_D3Dpp.FullScreen_RefreshRateInHz = refresh_rate_required;
        g_D3Dpp.BackBufferWidth  = m_ResolutionX;
        g_D3Dpp.BackBufferHeight = m_ResolutionY;
        g_D3Dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
        g_D3Dpp.hDeviceWindow = g_Wnd;		

		if (SUCCEEDED(g_D3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_D3Dpp.BackBufferFormat,
			g_D3Dpp.Windowed, expectedMultiSampleType, NULL)))
		{
			g_D3Dpp.MultiSampleType = expectedMultiSampleType;			
		}

        SETFLAG(g_Flags, GFLAG_STENCILAVAILABLE);


        if (D3D_OK != g_D3DD->Reset(&g_D3Dpp))
        {
            if (bpp == 16) g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D16;
            else g_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24X8;

            if (D3D_OK != g_D3DD->Reset(&g_D3Dpp)) ERROR_S(L"Sorry, unable to set this params.");
            RESETFLAG(g_Flags, GFLAG_STENCILAVAILABLE);
        }


	D3DVIEWPORT9 ViewPort;
	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

    ViewPort.X      = 0;
	ViewPort.Y      = 0;
    ViewPort.Width  = m_ResolutionX;
	ViewPort.Height = m_ResolutionY;

	ViewPort.MinZ   = 0.0f;
	ViewPort.MaxZ   = 1.0f;

	
	ASSERT_DX(g_D3DD->SetViewport(&ViewPort));
   

	S3D_Default();

	g_D3DD->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, (g_D3Dpp.MultiSampleType != D3DMULTISAMPLE_NONE));

	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl,sizeof(D3DMATERIAL9));
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
	light.Diffuse.r  = GetColorR(g_MatrixMap->m_LightMainColorObj);
	light.Diffuse.g  = GetColorG(g_MatrixMap->m_LightMainColorObj);
	light.Diffuse.b  = GetColorB(g_MatrixMap->m_LightMainColorObj);
	light.Ambient.r  = 0.0f;
	light.Ambient.g  = 0.0f;
	light.Ambient.b  = 0.0f;
	light.Specular.r = GetColorR(g_MatrixMap->m_LightMainColorObj);
	light.Specular.g = GetColorG(g_MatrixMap->m_LightMainColorObj);
	light.Specular.b = GetColorB(g_MatrixMap->m_LightMainColorObj);
	//light.Range    = 0;
	light.Direction	= g_MatrixMap->m_LightMain;
//	light.Direction=D3DXVECTOR3(250.0f,-50.0f,-250.0f);
//	D3DXVec3Normalize((D3DXVECTOR3 *)(&(light.Direction)),(D3DXVECTOR3 *)(&(light.Direction)));
	ASSERT_DX(g_D3DD->SetLight(0,&light));
	ASSERT_DX(g_D3DD->LightEnable(0,TRUE));

}

void MatrixGameDeinit(void)
{
    DTRACE();

    //Сохраняем указанное число шаблонов роботов игрока, если настройка MaxDesignsToSave не равна нулю
    int maxDesignsToSave = g_MatrixData->BlockGet(L"Config")->ParGet(CFG_DESIGNS_TO_SAVE).GetInt();

    if(maxDesignsToSave > 0)
    {
        //Если есть что сохранять (игрок построил хотя бы одного робота за матч)
        if(g_ConfigHistory->m_CurrentConfig != nullptr)
        {
            CBlockPar robotsCFG = CBlockPar(true, g_MatrixHeap);
            wchar robotsCFG_Path[MAX_PATH];
            SHGetSpecialFolderPathW(0, robotsCFG_Path, CSIDL_PERSONAL, true);
            wcscat(robotsCFG_Path, L"\\SpaceRangersHD\\RobotsCFG.txt");

            //Если обнаружили искомый конфиг, то загружаем его во избежание потери прочих возможно содержащихся в нём данных
            CWStr temp_path(g_MatrixHeap);
            if(CFile::FileExist(temp_path, robotsCFG_Path)) robotsCFG.LoadFromTextFile(robotsCFG_Path);
            //Иначе создаём совершенно новый объект документа
            else robotsCFG = CBlockPar(true, g_MatrixHeap);
            //Перед записью шаблонов удаляем возможно уже имевшийся там ранее блок
            robotsCFG.BlockDeleteNE(L"BotDesigns", 10);
            CBlockPar* bot_designs = robotsCFG.BlockAdd(L"BotDesigns");

            SRobotConfig* cur_config = g_ConfigHistory->m_LastConfig;

            int counter = maxDesignsToSave;
            //Начинаем перебор с конца списка шаблонов
            //И определяем "точку" начала отсчёта в списке шаблонов для сохранения
            while(counter > 1)
            {
                if(cur_config != g_ConfigHistory->m_FirstConfig && cur_config != nullptr)
                {
                    --counter;
                    cur_config = cur_config->m_PrevConfig;
                }
                else break;
            }

            CWStr to_config(g_MatrixHeap);
            counter = 0;
            while((cur_config != nullptr) && (counter < maxDesignsToSave))
            {
                to_config.Clear();

                //Формируем строку цифро-буквенного шаблона робота для записи в txt-конфиг
                //Определяем шасси
                auto cur_gear = cur_config->m_Chassis.m_nKind;
                if(cur_gear == RUK_CHASSIS_PNEUMATIC) to_config += L"P";
                else if(cur_gear == RUK_CHASSIS_WHEEL) to_config += L"W";
                else if(cur_gear == RUK_CHASSIS_TRACK) to_config += L"T";
                else if(cur_gear == RUK_CHASSIS_HOVERCRAFT) to_config += L"H";
                else if(cur_gear == RUK_CHASSIS_ANTIGRAVITY) to_config += L"A";

                //Определяем туловище
                cur_gear = cur_config->m_Hull.m_Unit.m_nKind;
                to_config += L",";
                if(cur_gear == RUK_ARMOR_6) to_config += L"1";
                else if(cur_gear == RUK_ARMOR_PASSIVE) to_config += L"1S";
                else if(cur_gear == RUK_ARMOR_ACTIVE) to_config += L"2";
                else if(cur_gear == RUK_ARMOR_FIREPROOF) to_config += L"2S";
                else if(cur_gear == RUK_ARMOR_PLASMIC) to_config += L"3";
                else if(cur_gear == RUK_ARMOR_NUCLEAR) to_config += L"4S";

                //Определяем оружие
                bool t_mark = false;
                for(int i = 0; i < MAX_WEAPON_CNT; ++i)
                {
                    cur_gear = cur_config->m_Weapon[i].m_nKind;
                    if(cur_gear)
                    {
                        if(!t_mark)
                        {
                            to_config += L",";
                            t_mark = true;
                        }
                        if(cur_gear == RUK_WEAPON_MACHINEGUN) to_config += L"G";
                        else if(cur_gear == RUK_WEAPON_CANNON) to_config += L"C";
                        else if(cur_gear == RUK_WEAPON_MISSILE) to_config += L"M";
                        else if(cur_gear == RUK_WEAPON_FLAMETHROWER) to_config += L"F";
                        else if(cur_gear == RUK_WEAPON_LASER) to_config += L"L";
                        else if(cur_gear == RUK_WEAPON_PLASMA) to_config += L"P";
                        else if(cur_gear == RUK_WEAPON_ELECTRIC) to_config += L"E";
                        else if(cur_gear == RUK_WEAPON_REPAIR) to_config += L"R";
                        else if(cur_gear == RUK_WEAPON_MORTAR) to_config += L"O";
                        else if(cur_gear == RUK_WEAPON_BOMB) to_config += L"B";
                    }
                }

                //Определяем голову
                cur_gear = cur_config->m_Head.m_nKind;
                if(cur_gear) to_config += L",";
                if(cur_gear == RUK_HEAD_BLOCKER) to_config += L"S";
                else if(cur_gear == RUK_HEAD_DYNAMO) to_config += L"D";
                else if(cur_gear == RUK_HEAD_LOCKATOR) to_config += L"L";
                else if(cur_gear == RUK_HEAD_FIREWALL) to_config += L"F";
                else if(cur_gear == RUK_HEAD_RAPID) to_config += L"R";
                else if(cur_gear == RUK_HEAD_DESIGN) to_config += L"D";
                else if(cur_gear == RUK_HEAD_SPEAKER) to_config += L"P";

                //Заносим готовый шаблон в список сохранения
                bot_designs->ParAdd(L"Bot" + CWStr(counter+1), to_config);

                //Выбираем следующий конфиг
                cur_config = cur_config->m_NextConfig;

                ++counter;
            }
            //Записываем готовые шаблоны в txt-конфиг
            robotsCFG.SaveInTextFile(robotsCFG_Path);
        }
    }

    SSpecialBot::ClearAIRobotType();

    g_Config.Clear();

    if (g_Render)
    {
        HDelete(CRenderPipeline, g_Render, g_MatrixHeap);
        g_Render = NULL;
    }

    CMatrixHint::ClearAll();
	
	if(g_MatrixMap)
    {
		ASSERT(g_MatrixHeap);

		HDelete(CMatrixMapLogic,g_MatrixMap,g_MatrixHeap);
		g_MatrixMap=NULL;
	}

    if(g_MatrixData)
    {
        HDelete(CBlockPar,g_MatrixData,NULL);
        g_MatrixData = NULL;
    }

    CMatrixRobot::DestroyPneumaticData();

    if(g_IFaceList)
    {
		ASSERT(g_MatrixHeap);
		HDelete(CIFaceList, g_IFaceList, g_MatrixHeap);
        g_IFaceList = NULL;
    }

    if(g_ConfigHistory)
    {
		ASSERT(g_MatrixHeap);
		HDelete(CHistory, g_ConfigHistory, g_MatrixHeap);
        g_ConfigHistory = NULL;
    }
	
    if(CIFaceMenu::m_MenuGraphics)
    {
        HDelete(CInterface, CIFaceMenu::m_MenuGraphics, g_MatrixHeap);
        CIFaceMenu::m_MenuGraphics = NULL;
    }

    if(g_PopupMenu)
    {
        HDelete(CIFaceMenu, g_PopupMenu, g_MatrixHeap);
        g_PopupMenu = NULL;
    }
    
    if(g_PopupHead)
    {
        for(int i = 0; i < MENU_HEAD_ITEMS; ++i)
        {
            g_PopupHead[i].text.CWStr::~CWStr(); 
        }
        
        HFree(g_PopupHead, g_MatrixHeap);
        g_PopupHead = NULL;
    }

    if(g_PopupHull)
    {
        for(int i = 0; i < MENU_HULL_ITEMS; ++i)
        {
            g_PopupHull[i].text.CWStr::~CWStr();
        }
        
        HFree(g_PopupHull, g_MatrixHeap);
        g_PopupHull = NULL;
    }

    if(g_PopupWeaponNormal)
    {
        for(int i = 0; i < MENU_WEAPONNORM_ITEMS; ++i)
        {
            g_PopupWeaponNormal[i].text.CWStr::~CWStr();
        }
        
        HFree(g_PopupWeaponNormal, g_MatrixHeap);
        g_PopupWeaponNormal = NULL;
    }

    if(g_PopupWeaponExtern)
    {
        for(int i = 0; i < MENU_WEAPONEXTERN_ITEMS; ++i)
        {
            g_PopupWeaponExtern[i].text.CWStr::~CWStr();
        }
        
        HFree(g_PopupWeaponExtern, g_MatrixHeap);
        g_PopupWeaponExtern = NULL;
    }

    if(g_PopupChassis)
    {
        for(int i = 0; i < MENU_CHASSIS_ITEMS; ++i)
        {
            g_PopupChassis[i].text.CWStr::~CWStr();
        }
        
        HFree(g_PopupChassis, g_MatrixHeap);
        g_PopupChassis = NULL;
    }

    if(g_Config.m_Labels)
    {
        for(int i = 0;i < LABELS_LAST; ++i)
        {
            g_Config.m_Labels[i].CWStr::~CWStr();
        }
        HFree(g_Config.m_Labels, g_MatrixHeap);
        g_Config.m_Labels = NULL;
    }

    if(g_Config.m_Descriptions)
    {
        for(int i = 0;i < DESCRIPTIONS_LAST; ++i)
        {
            g_Config.m_Descriptions[i].CWStr::~CWStr();
        }
        HFree(g_Config.m_Descriptions, g_MatrixHeap);
        g_Config.m_Descriptions = NULL;
    }

    CInstDraw::ClearAll();
  
    CFile::ReleasePackFiles();

    if(g_MatrixHeap) 
    {
		HDelete(CHeap,g_MatrixHeap,NULL);
		g_MatrixHeap=NULL;
	}
}


LPCSTR PathToOutputFiles(LPSTR dest)
{
 ITEMIDLIST* pidl;
 static char lpPath[MAX_PATH];

 HRESULT hRes = SHGetSpecialFolderLocation( NULL, CSIDL_PERSONAL, &pidl );
 if (hRes==NOERROR) {
 SHGetPathFromIDList( pidl, lpPath );

 strcat(lpPath,"\\SpaceRangersHD");
 CreateDirectory(lpPath, NULL);
 strcat(lpPath,"\\");
 strcat(lpPath,dest);} else {strcpy(lpPath,"");}

 LPCSTR result;
 result=lpPath;

 //FILE * fp = fopen("1.log", "w+t");
 //fprintf(fp, "Direct3D=%s\n", result);
 //fclose(fp);

 return result;
}