#pragma once
#include <Windows.h>
#include <functional>
#include <string>
#include <vector>

#undef SOKU_EXPORTS

#ifdef SOKU_EXPORTS
#define SOKU_API	__declspec(dllexport)
#else
#define SOKU_API	__declspec(dllimport)
#define SOKU_MODULE	extern "C" __declspec(dllexport) void
#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#define IMGUI_API __declspec( dllimport )
#undef IMGUI_EXPORTS
#endif

#include "imgui.h"

#ifndef ANY_FUNCTION_H
#define ANY_FUNCTION_H

#include <cassert>      // For assert(...)
#include <memory>       // For std::unique_ptr<T>

struct any_function
{
public:
	struct type
	{
		const std::type_info* info;
		bool                                            is_lvalue_reference, is_rvalue_reference;
		bool                                            is_const, is_volatile;
		bool                                            operator == (const type& r) const { return info == r.info && is_lvalue_reference == r.is_lvalue_reference && is_rvalue_reference == r.is_rvalue_reference && is_const == r.is_const && is_volatile == r.is_volatile; }
		bool                                            operator != (const type& r) const { return !(*this == r); }
		template<class T> static type                   capture() { return{ &typeid(T), std::is_lvalue_reference<T>::value, std::is_rvalue_reference<T>::value, std::is_const<typename std::remove_reference<T>::type>::value, std::is_volatile<typename std::remove_reference<T>::type>::value }; }
	};

	class result
	{
		struct result_base
		{
			virtual                                     ~result_base() {}
			virtual std::unique_ptr<result_base>        clone() const = 0;
			virtual type                                get_type() const = 0;
			virtual void* get_address() = 0;
		};
		template<class T> struct typed_result : result_base
		{
			T                                           x;
			typed_result(T x) : x(get((void*)&x, tag<T>{})) {}
			std::unique_ptr<result_base>                clone() const { return std::unique_ptr<typed_result>(new typed_result(get((void*)&x, tag<T>{}))); }
			type                                        get_type() const { return type::capture<T>(); }
			void* get_address() { return (void*)&x; }
		};
		std::unique_ptr<result_base>                    p;
	public:
		result() {}
		result(result&& r) : p(move(r.p)) {}
		result(const result& r) { *this = r; }
		result& operator = (result&& r) { p.swap(r.p); return *this; }
		result& operator = (const result& r) { p = r.p ? r.p->clone() : nullptr; return *this; }

		type                                            get_type() const { return p ? p->get_type() : type::capture<void>(); }
		void* get_address() { return p ? p->get_address() : nullptr; }
		template<class T> T                             get_value() { assert(get_type() == type::capture<T>()); return get(p->get_address(), tag<T>{}); }

		template<class T> static result                 capture(T x) { result r; r.p.reset(new typed_result<T>(static_cast<T>(x))); return r; }
	};
	any_function() : result_type{} {}
	any_function(std::nullptr_t) : result_type{} {}
	template<class R, class... A>                       any_function(R(*p)(A...)) : any_function(p, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}
	template<class R, class... A>                       any_function(std::function<R(A...)> f) : any_function(f, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}
	template<class F>                                   any_function(F f) : any_function(f, &F::operator()) {}

	explicit                                            operator bool() const { return static_cast<bool>(func); }
	const std::vector<type>& get_parameter_types() const { return parameter_types; }
	const type& get_result_type() const { return result_type; }
	result                                              invoke(void* const args[]) const { return func(args); }
	result                                              invoke(std::initializer_list<void*> args) const { return invoke(args.begin()); }

private:
	template<class... T> struct                         tag {};
	template<std::size_t... IS> struct                  indices {};
	template<std::size_t N, std::size_t... IS> struct   build_indices : build_indices<N - 1, N - 1, IS...> {};
	template<std::size_t... IS> struct                  build_indices<0, IS...> : indices<IS...> {};

	template<class T> static T                          get(void* arg, tag<T>) { return           *reinterpret_cast<T*>(arg); }
	template<class T> static T& get(void* arg, tag<T&>) { return           *reinterpret_cast<T*>(arg); }
	template<class T> static T&& get(void* arg, tag<T&&>) { return std::move(*reinterpret_cast<T*>(arg)); }
	template<class F, class R, class... A, size_t... I> any_function(F f, tag<R   >, tag<A...>, indices<I...>) : parameter_types({ type::capture<A>()... }), result_type(type::capture<R   >()) { func = [f](void* const args[]) mutable { return result::capture<R>(f(get(args[I], tag<A>{})...));          }; }
	template<class F, class... A, size_t... I> any_function(F f, tag<void>, tag<A...>, indices<I...>) : parameter_types({ type::capture<A>()... }), result_type(type::capture<void>()) { func = [f](void* const args[]) mutable { return                    f(get(args[I], tag<A>{})...), result{}; }; }
	template<class F, class R                         > any_function(F f, tag<R   >, tag<    >, indices<    >) : parameter_types({}), result_type(type::capture<R   >()) { func = [f](void* const args[]) mutable { return result::capture<R>(f());          }; }
	template<class F                                  > any_function(F f, tag<void>, tag<    >, indices<    >) : parameter_types({}), result_type(type::capture<void>()) { func = [f](void* const args[]) mutable { return                    f(), result{}; }; }
	template<class F, class R, class... A             > any_function(F f, R(F::* p)(A...)) : any_function(f, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}
	template<class F, class R, class... A             > any_function(F f, R(F::* p)(A...) const) : any_function(f, tag<R>{}, tag<A...>{}, build_indices<sizeof...(A)>{}) {}

	std::function<result(void* const*)>               func;
	std::vector<type>                                   parameter_types;
	type                                                result_type;
};
#endif

#ifndef XS_PIMPL_OBJECT_H
#define XS_PIMPL_OBJECT_H

#ifdef _WIN32
#ifdef XS_PIMPL_EXPORTS
#define XS_PIMPL_API    __declspec( dllexport )
#else
#define XS_PIMPL_API    __declspec( dllimport )
#endif
#else
#define XS_PIMPL_API    
#endif

namespace Soku
{
	template< class T >
	class XS_PIMPL_API Object
	{
	public:
		/* C++11 support is buggy on MSVC V120 toolset (see C4520)... */
#if !defined( _MSC_FULL_VER ) || _MSC_FULL_VER >= 190024215
		Object(void);
		template< typename ... A >
		Object(A& ... a);
		template< typename ... A >
		Object(const A& ... a);
#else
		Object(void);
		template< typename A1, typename ... A2 >
		Object(A1 a1, A2 ... a2);
#endif
		Object(const Object< T >& o);
		Object(Object< T >&& o);
		virtual ~Object(void);
		Object< T >& operator =(Object< T > o);
		template< class U >
		friend void swap(Object< U >& o1, Object< U >& o2);
	private:
		friend T;
		class  IMPL;
		IMPL* impl;
	};
}
#endif

using LPDIRECT3DDEVICE9 = struct IDirect3DDevice9*;
using CallbackVoid = std::function<void(void)>;
using Callback = any_function; //std::function<void(void*)>;
using ConvertionFunc = std::function<char* (char*&, int&)>;
using PluginID = const std::string;
using EventID = int;
using FileID = int;
using ItemID = int;

using ptr_t = void*;
using lptr_t = DWORD*;
using addr_t = DWORD;
using ubyte = unsigned char;
using sbyte = char;
using uint = unsigned int;

enum class ComboModifier
{
	RiftAttack = 0x1,
	SmashAttack = 0x2,
	BorderResist = 0x4,
	ChainArts = 0x8,
	ChainSpell = 0x10,
	CounterHit = 0x20,
};

enum class ActionID
{
	STAND = 0x0,
	SITDOWN = 0x1,
	CROUCH = 0x2,
	STANDUP = 0x3,
	WALK_FORWARD = 0x4,
	WALK_BACKWARD = 0x5,
	JUMP_UP = 0x6,
	JUMP_FORWARD = 0x7,
	JUMP_BACKWARD = 0x8,
	A = 0x12D,
	BE2 = 0x0DC,
	BE3 = 0x0DD,
	BE1 = 0x0DE,
	BE6 = 0x0E0,
	BE4 = 0x0DF,
	aBE4 = 0x0E1,
	aBE6 = 0x0E2
};

enum class Character
{
	Reimu,
	Marisa,
	Sakuya,
	Alice,
	Patchouli,
	Youmu,
	Remilia,
	Yuyuko,
	Yukari,
	Suika,
	Reisen,
	Aya,
	Komachi,
	Iku,
	Tenshi,
	Sanae,
	Cirno,
	Meiling,
	Utsuho,
	Suwako,
	Random,
	Namazu,
};

enum class Weather
{
	Sunny,
	Drizzle,
	Cloudy,
	Blue_sky,
	Hail,
	Spring_haze,
	Heavy_fog,
	Snow,
	Sunshower,
	Sprinkle,
	Tempest,
	Mtn_vapor,
	River_mist,
	Typhoon,
	Calm,
	Diamond_dust,
	Dust_storm,
	Scorching_sun,
	Monsoon,
	Aurora,
	Twilight,
	Clear
};

enum class SpiritPower
{
	zero,
	one,
	two,
	three,
	four,
	five,
	normal
};

enum class State
{
	Stand,
	Crouch,
	Jump,
	P2Control
};

enum class Guard
{
	On,
	Off,
	High,
	Low,
	AfterOneHit
};

enum class TechingDirection
{
	Front,
	Back,
	Random,
	Off
};

enum class Position
{
	LeftCorner,
	p02,
	p03,
	p04,
	NearLeftCorner,
	p06,
	p07,
	Center,
	Center2,
	p10,
	p11,
	NearRightCorner,
	p13,
	p14,
	p15,
	RightCorner
};

enum class Align
{
	Undefined,
	NorthWest,
	North,
	NorthEast,
	West,
	Center,
	East,
	SouthWest,
	South,
	SouthEast
};

enum class Controller
{
	Player1,
	Player2,
	AI,
	AI2,
	Dummy,
	DummyBlock,
	Default
};

enum class MODE
{
	STORY = 0,
	ARCADE = 1,
	VSCOM = 2,
	VSPLAYER = 3,
	VSCLIENT = 4,
	VSSERVER = 5,
	VSWATCH = 6,
	PRACTICE = 8,
	REPLAY = 9,
	NONE = 10
};

enum class SCENE
{
	LOGO = 0,
	OPENING = 1,
	TITLE = 2,
	SELECT = 3,
	BATTLE = 5,
	LOADING = 6,
	SELECTSV = 8,
	SELECTCL = 9,
	LOADINGSV = 10,
	LOADINGCL = 11,
	LOADINGWATCH = 12,
	BATTLESV = 13,
	BATTLECL = 14,
	BATTLEWATCH = 15,
	SELECTSENARIO = 16,
	ENDING = 20,
	SUBMENU = 21
};

enum class MENU
{
	CUSTOM,
	VS_NETWORK,
	REPLAY,
	MUSICROOM,
	RESULT,
	PROFILE,
	PROFILE_V2,
	CONFIG,
	PRACTICE,
	PAUSE,
	NONE,
	UNKNOWN
};

enum class Stage
{
	RepairedHakureiShrine = 0x00,
	ForestOfMagic = 0x01,
	CreekOfGenbu = 0x02,
	YoukaiMountain = 0x03,
	MysteriousSeaOfClouds = 0x04,
	BhavaAgra = 0x05,
	HakureiShrine = 0x0A,
	KirisameMagicShop = 0x0B,
	SDMClockTower = 0x0C,
	ForestOfDolls = 0x0D,
	SDMLibrary = 0x0E,
	Netherworld = 0x0F,
	SDMFoyer = 0x10,
	HakugyokurouSnowyGarden = 0x11,
	BambooForestOfTheLost = 0x12,
	ShoreOfMistyLake = 0x1E,
	MoriyaShrine = 0x1F,
	MouthOfGeyser = 0x20,
	CatwalkInGeyser = 0x21,
	FusionReactorCore = 0x22
};

enum class Shadow
{
	Default,
	Normal,
	Reflected,
	None
};

enum class PlayerNum
{
	P1,
	P2
};

enum class BattleEvent
{
	GameStart = 0x0,
	RoundPreStart = 0x1,
	RoundStart = 0x2,
	RoundEnd = 0x3,
	GameEnd = 0x5,
	EndScreen = 0x6,
	ResultScreen = 0x7
};

enum class GameEvent
{
	SERVER_SETUP,
	SERVER_RELEASE,
	CONNECT_TO_SERVER,
	TITLE_CREATE,
	TITLE_PROCESS,
	BATTLE_MANAGER_CERATE,
	BATTLE_MANAGER_CLEAR,
	SUBMENU_PROCESS,
	SUBMENU_RELEASE,
	STORY_CREATE,
	ARCADE_CREATE,
	VS_COM_CREATE,
	VS_PLAYER_CREATE,
	VS_CLIENT_CREATE,
	VS_SERVER_CREATE,
	WATCH_CREATE,
	PRACTICE_CREATE,
	REPLAY_CREATE,
	RESULT_CREATE,
	UNKNOWN
};

enum class SokuComponent
{
	MainMenu,
	EngineMenu,
};

enum class SokuEvent
{
	Render,
	WindowProc,
	Keyboard,
	BattleEvent,
	GameEvent,
	StageSelect,
	FileLoader,
	Input,
	Debug,
	AtExit
};

enum class DataFormat
{
	RAW,
	PNG,
	CV0_DECRYPTED,
	CV1_DECRYPTED
};

struct functable
{
	int (WINAPI* destruct)(int a);
	int (WINAPI* read)(char* dest, int size);
	int (WINAPI* getRead)();
	void (WINAPI* seek)(int offset, int whence);
	int (WINAPI* getSize)();
};

enum class Font
{
	VeraMono,
	MonoSpatial,
	Peignot,
	//Ionicons
};

struct Inputs
{
	int axisX;
	int axisY;
	int A;
	int B;
	int C;
	int D;
	int CH;
	int S;
};

struct Texture
{
	ImTextureID		id;
	float			width;
	float			height;
};