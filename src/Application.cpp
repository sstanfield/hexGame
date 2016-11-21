#include "Application.h"
#include "animation/tb_animation.h"
#include "tb_font_renderer.h"
#include "tb_language.h"
#include "tb_message_window.h"
#include "tb_menu_window.h"
#include "tb_select_item.h"
#include "render/tbrenderer.h"
#include "settings.h"
#include "hexlua.h"

#include <iostream>

#ifdef TB_FONT_RENDERER_TBBF
	void register_tbbf_font_renderer();
#endif
#ifdef TB_FONT_RENDERER_STB
	void register_stb_font_renderer();
#endif
namespace hexgame {

static Application *g_gui_instance = nullptr;

Application::Application(std::unique_ptr<ServicesInterface> services, int width, int height) :
		services(std::move(services))
{
	g_gui_instance = this;
	renderer = std::make_unique<render::TBRenderer>();
	renderer->SetOpacity(1);
	tb::tb_core_init(renderer.get());
	// Load language file
	tb::g_tb_lng->Load((Settings::i()->getAssetDir()+"/lang/lng_en.tb.txt").c_str());
	if (tb::g_tb_skin->Load((Settings::i()->getAssetDir()+"/skin/skin.tb.txt").c_str(),
			(Settings::i()->getAssetDir()+"/skin/skin.hex.txt").c_str())) {
		std::cout << "Loaded skin" << std::endl;
	} else {
		std::cout << "Failed to load skin!" << std::endl;
		throw std::runtime_error("Failed to load skin!");
	}
#ifdef TB_FONT_RENDERER_TBBF
//	void register_tbbf_font_renderer();
	register_tbbf_font_renderer();
#endif
#ifdef TB_FONT_RENDERER_STB
//	void register_stb_font_renderer();
	register_stb_font_renderer();
#endif
	// Add fonts we can use to the font manager.
	//#if defined(TB_FONT_RENDERER_STB) || defined(TB_FONT_RENDERER_FREETYPE)
	//	g_font_manager->AddFontInfo("resources/vera.ttf", "Vera");
	//#endif
	tb::g_font_manager->AddFontInfo((Settings::i()->getAssetDir()+"fonts/DroidSans.ttf").c_str(), "Sans");
#ifdef TB_FONT_RENDERER_TBBF
	tb::g_font_manager->AddFontInfo((Settings::i()->getAssetDir()+"fonts/segoe_white_with_shadow.tb.txt").c_str(), "Segoe");
#endif

	// Set the default font description for widgets to one of the fonts we just added
	tb::TBFontDescription fd;
/*#ifdef TB_FONT_RENDERER_TBBF
	fd.SetID(TBIDC("Segoe"));
#else
	fd.SetID(TBIDC("Vera"));
#endif*/
	fd.SetID(TBIDC("Sans"));
	fd.SetSize(tb::g_tb_skin->GetDimensionConverter()->DpToPx(14));
	tb::g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	tb::TBFontFace *font = tb::g_font_manager->CreateFontFace(tb::g_font_manager->GetDefaultFontDescription());

	// Render some glyphs in one go now since we know we are going to use them. It would work fine
	// without this since glyphs are rendered when needed, but with some extra updating of the glyph bitmap.
	if (font)
		font->RenderGlyphs(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~•·åäöÅÄÖΣυρακούσα");
	root = std::make_unique<gui::RootWidget>();
	root->SetRect(tb::TBRect(0, 0, width, height));
	root->SetIsFocusable(true);
	root->SetFocus(tb::WIDGET_FOCUS_REASON_UNKNOWN);

	map = std::make_shared<state::Map>(100, 100, state::TileType::Grass);
	int row = 0;
	for (int col = 0; col < 100; col++)
		map->tile(row++, col).type = state::TileType::Mountain;
	int u1move = map->units.aquireMoveCost(2, 4, 5, 3, 4, 0, 0, 0, 1, 2, 1);
	state::Unit *u1 = map->units.newUnit(state::Side::Side_Black, "Test Cav", 3, 5, 20, u1move);
	map->placeUnit(u1, 50, 50);

	std::string layout = Settings::i()->getAssetDir()+"layouts/main.lua";
	lua::HexLua lua;
	lua.runScript(layout);
}

Application::~Application() {
	root = nullptr;  // Force delete before tb shutdown.
	tb::tb_core_shutdown();
	g_gui_instance = nullptr;
}

void Application::lostContext() {
	if (renderer) renderer->InvokeContextLost();
	glClose = true;
}

void Application::restoreContext(int width, int height) {
	if (renderer && glClose) renderer->InvokeContextRestored();
	root->SetRect(tb::TBRect(0, 0, width, height));
	glClose = false;
}

void Application::processFrame() {
	tb::TBMessageHandler::ProcessMessages();
	tb::TBAnimationManager::Update();
	root->InvokeProcessStates();
	root->InvokeProcess();
	tb::g_renderer->BeginPaint(root->GetRect().w, root->GetRect().h);
	root->InvokePaint(tb::TBWidget::PaintProps());
	tb::g_renderer->EndPaint();
}

void Application::setWindowSize(int width, int height) {
	root->SetRect(tb::TBRect(0, 0, width, height));
}

void Application::buttonPress(unsigned long button,
                 tb::MODIFIER_KEYS modifierkeys) {
	root->InvokePointerDown((int)(mouseX*scaleWidth),
	                        (int)(mouseY*scaleHeight), 1, modifierkeys, false);
}

void Application::buttonRelease(unsigned long button,
                   tb::MODIFIER_KEYS modifierkeys) {
	root->InvokePointerUp((int)(mouseX*scaleWidth), (int)(mouseY*scaleHeight),
	                      modifierkeys, false);
}

void Application::cursorPosition(double x, double y, tb::MODIFIER_KEYS modifierkeys) {
	mouseX = x;
	mouseY = y;
	root->InvokePointerMove((int)(mouseX*scaleWidth), (int)(mouseY*scaleHeight),
	                        modifierkeys, false);
}

void Application::scroll(double x, double y, tb::MODIFIER_KEYS modifierkeys) {
	root->InvokeWheel((int)(mouseX*scaleWidth), (int)(mouseY*scaleHeight),
	                  (int)x, -(int)y, modifierkeys);
}

Application *Application::instance() {
	return g_gui_instance;
}

// @return Return the upper case of a ascii charcter. Only for shortcut handling.
int Application::toupr_ascii(int ascii)
{
	if (ascii >= 'a' && ascii <= 'z')
		return ascii + 'A' - 'a';
	return ascii;
}

bool Application::InvokeShortcut(int key, tb::SPECIAL_KEY special_key,
                                   tb::MODIFIER_KEYS modifierkeys, bool down)
{
#ifdef TB_TARGET_MACOSX
	bool shortcut_key = (modifierkeys & tb::TB_SUPER) ? true : false;
#else
	bool shortcut_key = (modifierkeys & tb::TB_CTRL) ? true : false;
#endif
	if (!tb::TBWidget::focused_widget || !down || !shortcut_key)
		return false;
	bool reverse_key = (modifierkeys & tb::TB_SHIFT) ? true : false;
	int upper_key = toupr_ascii(key);
	tb::TBID id;
	if (upper_key == 'X')
		id = TBIDC("cut");
	else if (upper_key == 'C' || special_key == tb::TB_KEY_INSERT)
		id = TBIDC("copy");
	else if (upper_key == 'V' || (special_key == tb::TB_KEY_INSERT && reverse_key))
		id = TBIDC("paste");
	else if (upper_key == 'A')
		id = TBIDC("selectall");
	else if (upper_key == 'Z' || upper_key == 'Y') {
		bool undo = upper_key == 'Z';
		if (reverse_key)
			undo = !undo;
		id = undo ? TBIDC("undo") : TBIDC("redo");
	}
	else if (upper_key == 'N')
		id = TBIDC("new");
	else if (upper_key == 'O')
		id = TBIDC("open");
	else if (upper_key == 'S')
		id = TBIDC("save");
	else if (upper_key == 'W')
		id = TBIDC("close");
	else if (special_key == tb::TB_KEY_PAGE_UP)
		id = TBIDC("prev_doc");
	else if (special_key == tb::TB_KEY_PAGE_DOWN)
		id = TBIDC("next_doc");
	else
		return false;

	tb::TBWidgetEvent ev(tb::EVENT_TYPE_SHORTCUT);
	ev.modifierkeys = modifierkeys;
	ev.ref_id = id;
	return tb::TBWidget::focused_widget->InvokeEvent(ev);
}

bool Application::InvokeKey(unsigned int key, tb::SPECIAL_KEY special_key,
                              tb::MODIFIER_KEYS modifierkeys, bool down)
{
	if (InvokeShortcut(key, special_key, modifierkeys, down))
		return true;
	bool ret = root->InvokeKey(key, special_key, modifierkeys, down);
	if (!ret && root->isDimmed()) {
		// If root is dimmed this gives root level hotkeys as well as event
		// listeners (i.e. the main menu) a chance to work.
		tb::TBWidgetEvent ev(down ? tb::EVENT_TYPE_KEY_DOWN : tb::EVENT_TYPE_KEY_UP);
		ev.key = key;
		ev.special_key = special_key;
		ev.modifierkeys = modifierkeys;
		ret = root->InvokeEvent(ev);
	}
	return ret;
}

}  // hexgame

