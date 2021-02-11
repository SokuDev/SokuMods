#pragma once
#include "SokuLib-util.h"

namespace Soku
{
	using byte = unsigned char;
	class SOKU_API CPlayer : Object<CPlayer>
	{
	public:
		lptr_t			PTR() const;
		addr_t			ADDR() const;
		PlayerNum		Num() const;
		bool			IsInited() const;
		int				Port() const;
		void			Port(int port);
		short			HP() const;
		bool			HP(short hpValue);
		Inputs			GetInputs() const;
		Inputs			GetInputsEx() const;
		bool			SetInputs(Inputs& inputs);
		Character		CharID() const;
		std::string		CharStr() const;
		std::string		Name() const;
		bool			SetController(Controller contr);
		addr_t			GetDefController() const;
		int				Action(ActionID ID) const;
		int				Animation(ActionID ID) const;
		short			PrivateSquareDuration() const;
		bool			PrivateSquareDuration(short duration);
		short			MissingPowerDuration() const;
		bool			MissingPowerDuration(short duration);
		sbyte			DirectionFactor() const;
		byte			AirdashesCounter() const;
		short			UntechFrames() const;
		short			HitStopFrames() const;
		ActionID		CurrentActionID() const;
		CPlayer& Enemy() const;
		byte			DummyState() const;
		bool			IsEnemyCastsSpellcard() const; //BE is not allowed time
		bool			IsGettingHit() const;
		bool			IsLimit() const;
		bool			IsGuard() const;
		bool			IsBlocked() const;
		bool			IsBlockedOnGround() const;
		bool			IsBlockedHigh() const;
		bool			IsBlockedLow() const;
		bool			IsAirBlocked() const;
		bool			IsWakeUp() const;
		bool			IsOnGround() const;
		//byte			CardsInHandCount() const;
		//byte			CardsInHandCountMax() const;
		//int			CardsInDeckCount() const;
		//ptr_t			DeckData() const;
		//bool			InLimit() const;
		//byte			ComboModifierFlag() const;
		//short			ComboHitCount() const;
		//float			ComboRate() const;
		//short			ComboDamage() const;
		//short			ComboLimitCount() const;
		//short			StopwatchTimer() const;
		//float			MagicPotionTimer() const;
		//short			DragonStarTimer() const;
		//short			ThreeHeavenlyDropsTimer() const;
		//short			IllnessRecoveryCharmTimer() const;
		//short			UnderTimeStopEffectTimer() const;
		//short			UnderReisen1scTimer() const;
		//short			UnderTenshi1scTimer() const;
		//short			MaxHP() const;
		//Weather		CurrentWeather() const;
		//float			RiverMistPushRate() const;
		//bool			UnderCalm() const;
		//bool			UnderSpringHaze() const;
		//bool			UnderSunShower() const;
		//bool			UnderSprinkle() const
		//bool			UnderMoonson() const;
		//bool			UnderTyphoon() const;
		//bool			UnderMtnVapor() const;
		//bool			UnderDustStorm() const;
		//short			CalmHealValue() const;
		//float			SnowModifer();
		//float			GaugeModifer();
		//float			SpelcardDamageModifer() const;
		//float			SpecialMovesDamageModifer() const;
		//float			DamageModifer() const;
		//float			LifestealModifer() const;
		//short			InvulCardLevel() const;
		//float			CoinPushbackRate() const;
		//short			TenguFanLevel() const;
		//float			WalkAndAirddashesSpeedRate();
		//short			SpiritMax();
		//short			Spirit();
		//short			AviableOrbs();
		//short			CrushedOrbs();
		//bool			IsRightblocked();
		/*internal funcs*/
		void			_INIT(lptr_t player_ptr, PlayerNum player_num);
		void			_RELEASE();
	};

	SOKU_API CPlayer& Player(PlayerNum num);
	SOKU_API extern CPlayer P1;
	SOKU_API extern CPlayer P2;

	/*##############################################################################################################################
	##############################################################################################################################*/

	class SOKU_API CWeatherManager : Object<CWeatherManager>
	{
	public:
		Weather			CurrentWeather() const;
		Weather			NextWeather() const;
		bool			SetWeather(Weather weatherID);
		bool			SetWeather(Weather weatherID, short time);
		bool			SetNextWeather(Weather weatherID);
		short			Timer() const;
		void			Timer(short frames);
		bool			IsClear() const;
		bool			IsInited() const;
	};

	SOKU_API CWeatherManager& WeatherManager();
	SOKU_API extern CWeatherManager weatherManager;

	/*##############################################################################################################################
	##############################################################################################################################*/

	class SOKU_API CBattleManager : Object<CBattleManager>
	{
	public:
		addr_t	ADDR() const;
		lptr_t	PTR() const;
		lptr_t	P1_PTR() const;
		lptr_t	P2_PTR() const;
		bool	IsInited() const;
		uint	GameTime() const;
		/*internal funcs*/
		void	_INIT(ptr_t battle_manager_ptr);
		void	_DEINIT();
	};

	SOKU_API CBattleManager& BattleManager();
	SOKU_API extern CBattleManager battleManager;

	/*##############################################################################################################################
	##############################################################################################################################*/

	class SOKU_API CServer : Object<CServer>
	{
	public:
		addr_t	ADDR() const;
		addr_t	NET_OBJ_ADDR() const;
		bool	IsInited() const;
		int		Port() const;
		bool	IsSpecAllowed() const;
		bool	IsClientJoined() const;
		char* ClientName() const;
		char* ServerName() const;
		bool	Host(bool async);
		void	UnHost();
		bool	IsClientJoinedCustom() const;
		/*internal funcs*/
		void	_INIT(addr_t server_addr);
		void	_RELEASE();
	};

	SOKU_API CServer& Server();
	SOKU_API extern CServer server;

	/*##############################################################################################################################
	##############################################################################################################################*/

	class SOKU_API CPracticeConfig : Object<CPracticeConfig>
	{
	public:
		Weather				Weather() const;
		void				Weather(::Weather);
		SpiritPower			SpiritPower() const;
		void				SpiritPower(::SpiritPower);
		State				State() const;
		void				State(::State);
		Position			Position() const;
		void				Position(::Position);
		Guard				Guard() const;
		void				Guard(::Guard);
		bool				Counter() const;
		void				Counter(bool);
		TechingDirection	TechingDirection() const;
		void				TechingDirection(::TechingDirection);
	};

	SOKU_API CPracticeConfig PracticeConfig();

	/*##############################################################################################################################
	##############################################################################################################################*/

	class SOKU_API Module : Object<Module>
	{
	public:
		std::wstring	Path() const;
		std::wstring	Name() const;
		bool			IsInjected() const;
		bool			IsEnabled() const;
		bool			Inject();
		bool			Eject();
		Module(int id);
	};

	SOKU_API std::vector<Module>	GetModuleList();
	SOKU_API Module					GetModule();

	/*##############################################################################################################################
	##############################################################################################################################*/

	SOKU_API EventID		SubscribeEvent(SokuEvent event, Callback&& func);
	SOKU_API EventID		SubscribeEvent(SokuEvent event, Callback& func);
	//SOKU_API EventID		SubscribeEvent(SokuEvent event, void(*callback)(int&));
	//SOKU_API EventID		SubscribeEvent(SokuEvent event, CallbackVoid&& func);
	SOKU_API bool			UnsubscribeEvent(SokuEvent event);
	SOKU_API bool			UnsubscribeEvent(EventID ID);
	SOKU_API bool			UnsubscribeAllEvents();

	SOKU_API FileID				AddFile(const std::wstring& path);
	SOKU_API bool				RemoveFile(const std::wstring& path);
	SOKU_API bool				RemoveFile(FileID ID);
	SOKU_API bool				RemoveAllFiles();
	SOKU_API void				AddCustomConverter(std::string in, std::string out, ConvertionFunc&& func);
	SOKU_API void				RemoveAllCustomConverters();
	SOKU_API std::vector<std::wstring> GetFileList();

	SOKU_API ItemID			AddItem(SokuComponent component, const std::string& title, CallbackVoid&& func);
	SOKU_API ItemID			AddItem(SokuComponent component, CallbackVoid&& func);
	SOKU_API bool			RemoveItem(ItemID ID);
	SOKU_API bool			RemoveAllItems();

	/*##############################################################################################################################
	##############################################################################################################################*/

	class SOKU_API Image : Object<Image>
	{
	public:
		void		Width(float w);
		float		Width() const;
		void		Height(float h);
		float		Height() const;
		void		Delay(int delay);
		int			Delay() const;
		void		CurrentFrame(int num);
		int			CurrentFrame() const;
		void		TotalFrames(int num);
		int			TotalFrames() const;
		int			LoadedFrames() const;
		int			Delay—ounter() const;
		void		AddImage(ImTextureID tex);
		void		Display(ImVec2& size, ImVec2& pos);
		void		Reset();
		ImTextureID	Texture();
		void		Animate();
	};

	SOKU_API bool			CreateTextureFromFile(const std::wstring& path, Texture* tex);
	SOKU_API bool			CreateTextureFromResource(HMODULE hModule, int resName, Texture* tex);
	SOKU_API bool			CreateImageFromFile(const std::wstring& path, Image* image);
	SOKU_API bool			CreateImageFromResource(HMODULE hModule, int resName, Image* image);
	SOKU_API void			ExtractResource(HMODULE hModule, int strCustomResName, int nResourceId, const std::wstring& strOutputName);
	SOKU_API bool			CreateMenuImage(const std::wstring& path, ImTextureID* texMenu, ImTextureID* texMenuHover);

	/*##############################################################################################################################
	##############################################################################################################################*/

	SOKU_API bool			SetWeather(Weather weatherID);
	SOKU_API int			RunAction(ptr_t player, ActionID ID);
	SOKU_API int			RunAnimation(ptr_t player, ActionID ID);
	SOKU_API bool			ReadProfile(std::string profileName, byte r = 0xA0, byte g = 0xA0, byte b = 0xFF);
	SOKU_API void			PlaySE(int id);
	SOKU_API void			SetMainMenu(std::vector<std::string> menu);

	SOKU_API void			SetShadow(Shadow ID);
	SOKU_API std::string	ShadowIdToStr(Shadow ID);
	SOKU_API std::string	StageIdToStr(Stage ID);
	SOKU_API std::string	CharIdToStr(Character ID);

	SOKU_API LPDIRECT3DDEVICE9	D3DDevice();
	SOKU_API bool			IsWindowed();
	SOKU_API bool			IsRoll();
	SOKU_API HWND			HWnd();
	SOKU_API int			MenuAnimationState();
	SOKU_API int			BlackScreenOpacity();
	SOKU_API MODE			GameMode();
	SOKU_API SCENE			GameScene();
	SOKU_API MENU			GameMenu();
}

namespace ImGui
{
	IMGUI_API bool			WindowBegin(const char* name, Align align = Align::Undefined, const ImVec2& offset = ImVec2(0.0f, 0.0f), const ImVec4& col = ImVec4(0.0f, 0.0f, 0.0f, 1.0f), float bg_alpha = 1.0f, ImGuiWindowFlags flags = 0);
	IMGUI_API void			WindowEnd();
	IMGUI_API void			Spacing(float spacing = -1.0f);
	IMGUI_API ImFont* AddFontFromFile(const char* filename, float size = 13.0f);
	IMGUI_API ImFont* AddFontFromResource(Font font, float size = 13.0f, const ImWchar* glyph_ranges = (const ImWchar*)0);
	IMGUI_API void			Text(ImFont* font, float font_offset, ImVec4& bg_col, float bg_offset, const char* fmt, ...);
	IMGUI_API void 			Text(ImFont* font, ImVec4& bg_col, const char* fmt, ...);
	IMGUI_API bool			ImageAnimButton(const char* label, ImTextureID button_texture, ImTextureID button_texture_hovered, ImTextureID button_texture_activated, const ImVec2& size, bool activated, float frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec2& uv_a = ImVec2(0.0f, 0.0f), const ImVec2& uv_b = ImVec2(1.0f, 1.0f));
	IMGUI_API void			HelpMarker(const char* desc);
	IMGUI_API void			Text(int value);
}

namespace SokuData
{
	struct InputData
	{
		Soku::CPlayer* player;
		Inputs* inputs;
	};

	struct GameEventData
	{
		GameEvent		event;
		ptr_t			ptr;
	};

	struct FileLoaderData
	{
		const char* fileName;
		ptr_t           data;
		int             size;
		functable* reader;
		DataFormat		inputFormat;
	};

	struct RenderData
	{
		SCENE			scene;
		MODE			mode;
		MENU			menu;
	};

	struct KeyboardData
	{
		WPARAM			key;
		LPARAM			lParam;
	};

	struct WindowProcData
	{
		UINT			uMsg;
		WPARAM			wParam;
		LPARAM			lParam;
	};

	struct BattleEventData
	{
		BattleEvent		event;
	};

	struct StageData
	{
		Stage& stage;
	};
}