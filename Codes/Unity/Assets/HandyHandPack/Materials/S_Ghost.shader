 
Shader "Custom/Ghost" {
Properties {
    _Color ("Main Color", Color) = (1,1,1,1)
    _MainTex ("Base (RGB) Trans (A)", 2D) = "white" {}
    _BumpMap ("Normal Map", 2D) = "bump" {}


   _FresnelColor ("Fresnel Color", Color) = (1,1,1,1)
   [PowerSlider(4)] _FresnelExponent ("Fresnel Exponent", Range(0.25, 4)) = 1
}
 
SubShader {
    Tags {"RenderType"="Transparent" "Queue"="Transparent" "IgnoreProjector"="True"}
    LOD 200
   
    Pass {
        ZWrite On
        ColorMask 0
   
        CGPROGRAM
        #pragma vertex vert
        #pragma fragment frag
        #include "UnityCG.cginc"
 
        struct v2f {
            float4 pos : SV_POSITION;
        };
 
        v2f vert (appdata_base v)
        {
            v2f o;
            o.pos = UnityObjectToClipPos (v.vertex);
            return o;
        }
 
        half4 frag (v2f i) : COLOR
        {
            return half4 (0,0,0,0);
        }
        ENDCG  
    }
   
    CGPROGRAM
    #pragma surface surf Lambert alpha
    #pragma debug
	

    struct Input {
        float2 uv_MainTex;
	float3 worldNormal;
        float3 viewDir;
	float2 uv_BumpMap;
        INTERNAL_DATA
    };

    sampler2D _MainTex;
    fixed4 _Color;
    sampler2D _BumpMap;
    
    half3 _Emission;

    float3 _FresnelColor;
    float _FresnelExponent;
 
 
	void surf (Input IN, inout SurfaceOutput o) {
		half4 c = tex2D (_MainTex, IN.uv_MainTex);
		o.Normal = UnpackNormal(tex2D(_BumpMap, IN.uv_BumpMap));

			//get the dot product between the normal and the view direction
		float fresnel = dot(normalize(IN.viewDir),o.Normal);
			//invert the fresnel so the big values are on the outside
		fresnel = saturate(1 - fresnel);
			//raise the fresnel value to the exponents power to be able to adjust it
		fresnel = pow(fresnel, _FresnelExponent);
			//combine the fresnel value with a color
		float3 fresnelColor = fresnel * _FresnelColor;
			//apply the fresnel value to the emission
		o.Albedo = (c.r + c.g + c.b)/3;;
		o.Emission.rgb = _Emission + fresnelColor;
		o.Alpha = _Emission + fresnelColor;

        
	}
	ENDCG
}
 
Fallback "Transparent/Diffuse"
}