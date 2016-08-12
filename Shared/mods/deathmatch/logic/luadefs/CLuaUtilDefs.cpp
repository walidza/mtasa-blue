/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/mods/logic/luadefs/CLuaUtilDefs.cpp
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void CLuaUtilDefs::LoadFunctions ( void )
{
    // Util functions to make scripting easier for the end user
    // Some of these are based on standard mIRC script funcs as a lot of people will be used to them
    CLuaCFunctions::AddFunction ( "deref", Dereference );
    CLuaCFunctions::AddFunction ( "ref", Reference );
    CLuaCFunctions::AddFunction ( "getTickCount", GetTickCount_ );
    CLuaCFunctions::AddFunction ( "getRealTime", GetCTime );
    CLuaCFunctions::AddFunction ( "split", Split );
    CLuaCFunctions::AddFunction ( "isOOPEnabled", IsOOPEnabled );
    CLuaCFunctions::AddFunction ( "getUserdataType", GetUserdataType );
    CLuaCFunctions::AddFunction ( "print", luaB_print );
    CLuaCFunctions::AddFunction ( "getColorFromString", GetColorFromString );

    // Utility vector math functions
    CLuaCFunctions::AddFunction ( "getDistanceBetweenPoints2D", GetDistanceBetweenPoints2D );
    CLuaCFunctions::AddFunction ( "getDistanceBetweenPoints3D", GetDistanceBetweenPoints3D );
    CLuaCFunctions::AddFunction ( "getEasingValue", GetEasingValue );
    CLuaCFunctions::AddFunction ( "interpolateBetween", InterpolateBetween );

    // JSON funcs
    CLuaCFunctions::AddFunction ( "toJSON", toJSON );
    CLuaCFunctions::AddFunction ( "fromJSON", fromJSON );

    // PCRE functions
    CLuaCFunctions::AddFunction ( "pregFind", PregFind );
    CLuaCFunctions::AddFunction ( "pregReplace", PregReplace );
    CLuaCFunctions::AddFunction ( "pregMatch", PregMatch );

    // Package functions
    CLuaCFunctions::AddFunction ( "require", Require );
}

int CLuaUtilDefs::DisabledFunction ( lua_State* luaVM )
{
    m_pScriptDebugging->LogError ( luaVM, "Unsafe function was called." );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::Dereference ( lua_State* luaVM )
{
    int iPointer = 0;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( iPointer );

    if ( !argStream.HasErrors () )
    {
        lua_getref ( luaVM, iPointer );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::Reference ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );

    if ( !argStream.NextIsNil () && !argStream.NextIsNone () )
    {
        int iPointer = lua_ref ( luaVM, 1 );
        lua_pushnumber ( luaVM, iPointer );
        return 1;
    }
    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::GetTickCount_ ( lua_State* luaVM )
{
    double dTime = static_cast < double > ( GetTickCount64_ () );
    lua_pushnumber ( luaVM, dTime );
    return 1;
}

int CLuaUtilDefs::GetCTime ( lua_State* luaVM )
{
    // table getRealTime( [int seconds = current], bool localTime = true )
    time_t timer;
    time ( &timer );
    bool bLocalTime = true;
    CScriptArgReader argStream ( luaVM );

    if ( argStream.NextCouldBeNumber () )
    {
        argStream.ReadNumber ( timer );
        if ( timer < 0 )
        {
            argStream.SetCustomError ( "seconds cannot be negative" );
        }
    }

    if ( argStream.NextIsBool () )
        argStream.ReadBool ( bLocalTime );

    tm * time;
    if ( bLocalTime )
        time = localtime ( &timer );
    else
        time = gmtime ( &timer );

    if ( time == NULL )
        argStream.SetCustomError ( "seconds is out of range" );

    if ( argStream.HasErrors () )
    {
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    CLuaArguments ret;
    ret.PushString ( "second" );
    ret.PushNumber ( time->tm_sec );
    ret.PushString ( "minute" );
    ret.PushNumber ( time->tm_min );
    ret.PushString ( "hour" );
    ret.PushNumber ( time->tm_hour );
    ret.PushString ( "monthday" );
    ret.PushNumber ( time->tm_mday );
    ret.PushString ( "month" );
    ret.PushNumber ( time->tm_mon );
    ret.PushString ( "year" );
    ret.PushNumber ( time->tm_year );
    ret.PushString ( "weekday" );
    ret.PushNumber ( time->tm_wday );
    ret.PushString ( "yearday" );
    ret.PushNumber ( time->tm_yday );
    ret.PushString ( "isdst" );
    ret.PushNumber ( time->tm_isdst );
    ret.PushString ( "timestamp" );
    ret.PushNumber ( (double) timer );

    ret.PushAsTable ( luaVM );

    return 1;
}

int CLuaUtilDefs::Split ( lua_State* luaVM )
{
    SString strInput = "";
    unsigned int uiDelimiter = 0;
    SString strDelimiter;
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strInput );

    if ( argStream.NextIsNumber () )
    {
        argStream.ReadNumber ( uiDelimiter );
        wchar_t wUNICODE[2] = { uiDelimiter, '\0' };
        strDelimiter = UTF16ToMbUTF8 ( wUNICODE );
    }
    else  // It's already a string
        argStream.ReadString ( strDelimiter );

    if ( !argStream.HasErrors () )
    {
        // Copy the string
        char* strText = new char[strInput.length () + 1];
        strcpy ( strText, strInput );

        unsigned int uiCount = 0;
        char* szToken = strtok ( strText, strDelimiter );

        // Create a new table
        lua_newtable ( luaVM );

        // Add our first token
        lua_pushnumber ( luaVM, ++uiCount );
        lua_pushstring ( luaVM, szToken );
        lua_settable ( luaVM, -3 );

        // strtok until we're out of tokens
        while ( szToken = strtok ( NULL, strDelimiter ) )
        {
            // Add the token to the table
            lua_pushnumber ( luaVM, ++uiCount );
            lua_pushstring ( luaVM, szToken );
            lua_settable ( luaVM, -3 );
        }

        // Delete the text
        delete[] strText;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    return 1;
}

int CLuaUtilDefs::IsOOPEnabled ( lua_State* luaVM )
{
    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( luaVM );
    if ( pLuaMain )
        lua_pushboolean ( luaVM, pLuaMain->IsOOPEnabled () );
    else
        lua_pushnil ( luaVM );

    return 1;
}

int CLuaUtilDefs::GetUserdataType ( lua_State* luaVM )
{
    CScriptArgReader argStream ( luaVM );
    int iArgument = lua_type ( luaVM, 1 );

    if ( argStream.NextIsUserData () )
    {
        SString strType;
        if ( iArgument == LUA_TLIGHTUSERDATA )
            strType = GetUserDataClassName ( lua_touserdata ( luaVM, 1 ), luaVM, false );
        else if ( iArgument == LUA_TUSERDATA )
            strType = GetUserDataClassName ( *( (void**) lua_touserdata ( luaVM, 1 ) ), luaVM, false );

        strType = strType.empty () ? "userdata" : strType;

        lua_pushstring ( luaVM, strType.c_str () );
        return 1;
    }
    else
    {
        argStream.SetCustomError ( SString ( "Bad Argument: Expected userdata, got " + EnumToString ( (eLuaType) iArgument ) ) );
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
    }

    lua_pushboolean ( luaVM, false );
    return 1;
}

/* Modified from Lua's own print */
int CLuaUtilDefs::luaB_print ( lua_State *L ) {
    int n = lua_gettop ( L );  /* number of arguments */
    int i;
    SString output = "";
    lua_getglobal ( L, "tostring" );
    for ( i = 1; i <= n; i++ ) {
        const char *s;
        lua_pushvalue ( L, -1 );  /* function to be called */
        lua_pushvalue ( L, i );   /* value to print */
        lua_call ( L, 1, 1 );
        s = lua_tostring ( L, -1 );  /* get result */
        if ( s == NULL )
            return luaL_error ( L, LUA_QL ( "tostring" ) " must return a string to "
                LUA_QL ( "print" ) );
        if ( i>1 ) output += "    ";
        output += s;
        lua_pop ( L, 1 );  /* pop result */
    }
    m_pScriptDebugging->LogInformation ( L, "%s", output.c_str () );
    return 0;
}

int CLuaUtilDefs::GetColorFromString ( lua_State* luaVM )
{
    //  int int int int getColorFromString ( string theColor )
    SString strColor;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strColor );

    if ( !argStream.HasErrors () )
    {
        unsigned char ucColorRed, ucColorGreen, ucColorBlue, ucColorAlpha;
        if ( XMLColorToInt ( strColor, ucColorRed, ucColorGreen, ucColorBlue, ucColorAlpha ) )
        {
            lua_pushnumber ( luaVM, ucColorRed );
            lua_pushnumber ( luaVM, ucColorGreen );
            lua_pushnumber ( luaVM, ucColorBlue );
            lua_pushnumber ( luaVM, ucColorAlpha );
            return 4;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::GetDistanceBetweenPoints2D ( lua_State* luaVM )
{
    //  float getDistanceBetweenPoints2D ( float x1, float y1, float x2, float y2 )
    CVector2D vecA, vecB;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadVector2D ( vecA );
    argStream.ReadVector2D ( vecB );

    if ( !argStream.HasErrors () )
    {
        // Return the distance
        lua_pushnumber ( luaVM, DistanceBetweenPoints2D ( vecA, vecB ) );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::GetDistanceBetweenPoints3D ( lua_State* luaVM )
{
    //  float getDistanceBetweenPoints3D ( float x1, float y1, float z1, float x2, float y2, float z2 )
    CVector vecA, vecB;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadVector3D ( vecA );
    argStream.ReadVector3D ( vecB );

    if ( !argStream.HasErrors () )
    {
        // Return the distance
        lua_pushnumber ( luaVM, DistanceBetweenPoints3D ( vecA, vecB ) );
        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::GetEasingValue ( lua_State* luaVM )
{
    //  float getEasingValue( float fProgress, string strEasingType [, float fEasingPeriod, float fEasingAmplitude, float fEasingOvershoot] )
    float fProgress; CEasingCurve::eType easingType; float fEasingPeriod; float fEasingAmplitude; float fEasingOvershoot;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadNumber ( fProgress );
    argStream.ReadEnumString ( easingType );
    argStream.ReadNumber ( fEasingPeriod, 0.3f );
    argStream.ReadNumber ( fEasingAmplitude, 1.0f );
    argStream.ReadNumber ( fEasingOvershoot, 1.70158f );

    if ( argStream.HasErrors () )
    {
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    CEasingCurve easingCurve ( easingType );
    easingCurve.SetParams ( fEasingPeriod, fEasingAmplitude, fEasingOvershoot );
    lua_pushnumber ( luaVM, easingCurve.ValueForProgress ( fProgress ) );
    return 1;
}

int CLuaUtilDefs::InterpolateBetween ( lua_State* luaVM )
{
    //  float float float interpolateBetween ( float x1, float y1, float z1, 
    //      float x2, float y2, float z2, 
    //      float fProgress, string strEasingType, 
    //      [ float fEasingPeriod, float fEasingAmplitude, float fEasingOvershoot ] )
    CVector vecPointA; CVector vecPointB;
    float fProgress; CEasingCurve::eType easingType;
    float fEasingPeriod; float fEasingAmplitude; float fEasingOvershoot;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadVector3D ( vecPointA );
    argStream.ReadVector3D ( vecPointB );
    argStream.ReadNumber ( fProgress );
    argStream.ReadEnumString ( easingType );
    argStream.ReadNumber ( fEasingPeriod, 0.3f );
    argStream.ReadNumber ( fEasingAmplitude, 1.0f );
    argStream.ReadNumber ( fEasingOvershoot, 1.70158f );

    if ( argStream.HasErrors () )
    {
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
        lua_pushboolean ( luaVM, false );
        return 1;
    }

    CVector vecResult = TInterpolation < CVector >::Interpolate ( vecPointA, vecPointB, fProgress, easingType, fEasingPeriod, fEasingAmplitude, fEasingOvershoot );
    lua_pushnumber ( luaVM, vecResult.fX );
    lua_pushnumber ( luaVM, vecResult.fY );
    lua_pushnumber ( luaVM, vecResult.fZ );
    return 3;
}

int CLuaUtilDefs::toJSON ( lua_State* luaVM )
{
    // Got a string argument?
    CScriptArgReader argStream ( luaVM );

    if ( !argStream.NextIsNil () )
    {
        int jsonFlags = 0;
        // Read the argument
        CLuaArguments JSON;
        JSON.ReadArgument ( luaVM, 1 );
        argStream.Skip ( 1 );

        bool bCompact;
        argStream.ReadBool ( bCompact, false );
        jsonFlags |= bCompact ? JSON_C_TO_STRING_PLAIN : JSON_C_TO_STRING_SPACED;

        eJSONPrettyType jsonPrettyType;
        argStream.ReadEnumString ( jsonPrettyType, JSONPRETTY_NONE );
        if ( jsonPrettyType != JSONPRETTY_NONE )
            jsonFlags |= jsonPrettyType;

        if ( !argStream.HasErrors () )
        {
            // Convert it to a JSON string
            std::string strJSON;
            if ( JSON.WriteToJSONString ( strJSON, false, jsonFlags ) )
            {
                // Return the JSON string
                lua_pushstring ( luaVM, strJSON.c_str () );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );
    }
    else
        m_pScriptDebugging->LogBadType ( luaVM );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}

int CLuaUtilDefs::fromJSON ( lua_State* luaVM )
{
    // Got a string argument?
    SString strJson = "";
    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strJson );

    if ( !argStream.HasErrors () )
    {
        // Read it into lua arguments
        CLuaArguments Converted;
        if ( Converted.ReadFromJSONString ( strJson ) )
        {
            // Return it as data
            Converted.PushArguments ( luaVM );
            return Converted.Count ();
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    // Failed
    lua_pushnil ( luaVM );
    return 1;
}

int CLuaUtilDefs::PregFind ( lua_State* luaVM )
{
    //  bool pregFind ( string base, string pattern, uint/string flags = 0 )
    SString strBase, strPattern;
    pcrecpp::RE_Options pOptions;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strBase );
    argStream.ReadString ( strPattern );
    ReadPregFlags ( argStream, pOptions );

    if ( !argStream.HasErrors () )
    {
        pcrecpp::RE pPattern ( strPattern, pOptions );

        if ( pPattern.PartialMatch ( strBase ) )
        {
            lua_pushboolean ( luaVM, true );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::PregReplace ( lua_State* luaVM )
{
    //  string pregReplace ( string base, string pattern, string replace, uint/string flags = 0 )
    SString strBase, strPattern, strReplace;
    pcrecpp::RE_Options pOptions;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strBase );
    argStream.ReadString ( strPattern );
    argStream.ReadString ( strReplace );
    ReadPregFlags ( argStream, pOptions );

    if ( !argStream.HasErrors () )
    {
        pcrecpp::RE pPattern ( strPattern, pOptions );

        string strNew = strBase;
        if ( pPattern.GlobalReplace ( strReplace, &strNew ) )
        {
            lua_pushstring ( luaVM, strNew.c_str () );
            return 1;
        }
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::PregMatch ( lua_State* luaVM )
{
    //  table pregMatch ( string base, string pattern, uint/string flags = 0, int maxResults = 100000 )
    SString strBase, strPattern;
    pcrecpp::RE_Options pOptions;
    int iMaxResults;

    CScriptArgReader argStream ( luaVM );
    argStream.ReadString ( strBase );
    argStream.ReadString ( strPattern );
    ReadPregFlags ( argStream, pOptions );
    argStream.ReadNumber ( iMaxResults, 100000 );

    if ( !argStream.HasErrors () )
    {
        lua_newtable ( luaVM );

        pcrecpp::RE pPattern ( strPattern, pOptions );

        pcrecpp::StringPiece strInput ( strBase );

        string strGet; int i = 1;
        while ( pPattern.FindAndConsume ( &strInput, &strGet ) && i <= iMaxResults )
        {
            lua_pushnumber ( luaVM, i );
            lua_pushstring ( luaVM, strGet.c_str () );
            lua_settable ( luaVM, -3 );
            i++;
        }

        return 1;
    }
    else
        m_pScriptDebugging->LogCustom ( luaVM, argStream.GetFullErrorMessage () );

    lua_pushboolean ( luaVM, false );
    return 1;
}

int CLuaUtilDefs::Require(lua_State* luaVM)
{
#ifndef MTA_CLIENT
    //  table require ( string name )
    SString strMod;

    CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine(luaVM);
    CScriptArgReader argStream(luaVM);
    argStream.ReadString(strMod);

    if (!argStream.HasErrors())
    {
        // Check if package exists already, if so load it
                                                        // stack: ["moduleName"]
        pLuaMain->GetPackage(luaVM, strMod);            // stack: ["moduleName",pkgModule/nil]
        if (lua_type(luaVM, -1) != LUA_TNIL)
            return 1;
        lua_pop(luaVM,1);                               // stack: ["moduleName"]

        // Check whether the appropriate pure lua module exists
        if (pLuaMain->LoadLuaLib(luaVM, strMod))        // stack: ["moduleName",pkgLuaMod/nil]
            return 1;

        // Check if a C library exists
        if (pLuaMain->LoadClib(luaVM, strMod))          // stack: ["moduleName",fncModule/nil]
            return 1;
        lua_pop(luaVM,2);                               // stack: []
    }
    else
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
#endif
    lua_pushboolean(luaVM, false);
    return 1;
}