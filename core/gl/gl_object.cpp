#include "gl_object.h"
#include "../os/file_dir.h"
#include "../rt/small_math.h"
#include <algorithm>


namespace gl
{

static const LPCSTR vshPhong = 
	"SHADER_PRECISION(mediump,float)\n"
	"uniform mat4 _MVP;\n"
	"uniform mat3 _NormTrans;\n"
	"attribute vec4 _Position;\n"
	"attribute lowp vec3 _Normal;\n"
	"varying lowp vec3 f_Normal;\n"
	"#if defined(MATERIAL_PHONG_USE_KS) || defined(MATERIAL_USE_ALPHA)\n"
	"varying vec3 f_Position;\n"
	"#endif\n"
	//"attribute vec2 _TexCoord;\n"
	"void main()\n"
	"{\n"
	"	vec4 posvec = _MVP*_Position;\n"
	"	#if defined(MATERIAL_PHONG_USE_KS)\n"
	"	f_Position = vec3(posvec.x/posvec.w,posvec.y/posvec.w,posvec.z/posvec.w);\n"
	"	#endif\n"
	"	f_Normal = _NormTrans*(_Normal - vec3(0.5,0.5,0.5));\n"
	"	gl_Position = posvec;\n"
	"}";

static const LPCSTR fshPhong = 
	"SHADER_PRECISION(mediump,float)\n"
	"#if defined(MATERIAL_PHONG)\n"
	"uniform lowp float _MaterialData_Phong[11];\n"
	"#endif\n"
	"#if defined(MATERIAL_PHONG_USE_KS) || defined(MATERIAL_USE_ALPHA)\n"
	"varying vec3 f_Position;\n"
	"lowp vec3 norm;\n"
	"#endif\n"
	"varying lowp vec3 f_Normal;\n"
	//"uniform sampler2D _Texture;"
	//"varying vec2 texCoord;"
	"vec3 compute_shading(vec3 light)\n"
	"{\n"
	"	return vec3(0,0,0);\n"
	"}\n"

	"void main()\n"
	"{\n"
	"	vec3 light = 1.5*vec3(0.707,0,-0.707);\n"
	"	#if defined(MATERIAL_PHONG_USE_KD) || defined(MATERIAL_PHONG_USE_KS) || defined(MATERIAL_USE_ALPHA)\n"
	"	norm = normalize(f_Normal);\n"
	"	lowp float cosine = dot(norm, light);\n"
	"	#if defined(MATERIAL_PHONG_USE_KS) || defined(MATERIAL_USE_ALPHA)\n"
	"	vec3 view = normalize(f_Position)*-1.0;\n"
	"	float fresnel = exp(3.0*(1.0 - dot(view, norm)) - 3.0);\n"
	"	#endif\n"
	"	#endif\n"	
	"	vec3 intensity = vec3(0,0,0)\n"
	"		#if defined(MATERIAL_PHONG_USE_KA)\n"
	"		+ vec3(_MaterialData_Phong[0], _MaterialData_Phong[1], _MaterialData_Phong[2])\n"
	"		#endif\n"
	"		#if defined(MATERIAL_PHONG_USE_KD)\n"
	"		+ cosine*vec3(_MaterialData_Phong[3], _MaterialData_Phong[4], _MaterialData_Phong[5])\n"
	"		#endif\n"
	"		#if defined(MATERIAL_PHONG_USE_KD)\n"
	"		+ vec3(_MaterialData_Phong[6], _MaterialData_Phong[7], _MaterialData_Phong[8])*"
	"		  pow(dot(normalize(view + light), norm), _MaterialData_Phong[9])*(1.0 + fresnel)\n"
	"		#endif\n"
	"	;\n"
	//"texture2D(_Texture, texCoord);"
	//"{	gl_FragColor = textureLod(_Texture, texCoord, 0);"
	//"{	gl_FragColor = vec4(texCoord.x, texCoord.y, 0,0);"
	//"	gl_FragColor = fresnel;\n"
	"	#if defined(MATERIAL_USE_ALPHA)\n"
	"	gl_FragColor = vec4(intensity.x, intensity.y, intensity.z, fresnel + (1.0 - fresnel)*_MaterialData_Phong[10]);\n"
	"	#else\n"
	"	gl_FragColor = vec4(intensity.x, intensity.y, intensity.z, 1.0);\n"
	"	#endif\n"
	"}";

void Object::Destory()
{
}

int Surface::GetVertexAttributeSize() const
{
	if(!(_DataFlag&DF_POSITION_3F))return 0;
	static const int vasize[] = 
	{ 3*sizeof(float), 3*sizeof(float) + 3, 5*sizeof(float), 5*sizeof(float) + 3, 
	  3*sizeof(float) + 1, 3*sizeof(float) + 4, 5*sizeof(float) + 1, 5*sizeof(float) + 4
	};
	int vi = (_DataFlag&DF_ATTRIB_MASK)/2;
	if(vi > 7)return 0;
	return vasize[vi];
}

bool Surface::operator < (const Surface& s) const
{	
	if(_RenderOrder < s._RenderOrder)return true;
	return	_RenderOrder == s._RenderOrder &&
			_Center.L2NormSqr() < s._Center.L2NormSqr();
}

int Object::_GetShaderIndex(DWORD MaterialConfig)
{
	for(UINT i=0;i<_Shaders.GetSize();i++)
		if(_Shaders[i]._MaterialConfig == MaterialConfig)
			return i;

	UINT MaterialType = (MaterialConfig&MATERIAL_TYPE_MASK)>>MATERIAL_TYPE_SHIFT;
	// Create a shader
	if(MaterialType == MATERIAL_TYPE_PHONG)
	{
		MaterialShader& sh = _Shaders.push_back();
		sh._MaterialConfig = MaterialConfig;
		// set macros and compile 
		rt::String predefine("#define MATERIAL_PHONG\n");
		if(sh._MaterialConfig & MATERIAL_PHONG_USE_KA)predefine += "#define MATERIAL_PHONG_USE_KA\n";
		if(sh._MaterialConfig & MATERIAL_PHONG_USE_KD)predefine += "#define MATERIAL_PHONG_USE_KD\n";
		if(sh._MaterialConfig & MATERIAL_PHONG_USE_KS)predefine += "#define MATERIAL_PHONG_USE_KS\n";
		if(sh._MaterialConfig & MATERIAL_USE_ALPHA)predefine += "#define MATERIAL_USE_ALPHA\n";

		//_LOG(predefine);
		sh._Shader.SetPredefines(predefine);
		sh._Shader.BuildWithSourceCodes(vshPhong, fshPhong);

		return (int)_Shaders.GetSize()-1;
	}
	else
	{	return -1;
	}
}

bool Object::Load(LPCSTR filename)
{
	os::FilePacked_Reader	fp;
	Header	hdr;
	rt::Buffer<SurfaceChunk>	shdr;
	rt::Buffer<MaterialChunk>	mhdr;

	if(	fp.Open(filename) &&
		fp.Load(Header::FILENAME_HEADER, &hdr) &&
		hdr.Magic == Header::Magic_xScene &&
		fp.Load(Header::FILENAME_SURFACECHUNK, shdr) &&
		hdr.SurfaceCount == shdr.GetSize() &&
		_Surfaces.SetSize(hdr.SurfaceCount)
	)
	{	rt::Buffer<BYTE>	buf;
		for(UINT i=0;i<shdr.GetSize();i++)
		{
			_Surfaces[i]._MaterialName = shdr[i]._MaterialName;
			_Surfaces[i]._DataFlag = shdr[i]._DataFlag;
			_Surfaces[i]._TriangleCount = shdr[i]._TriangleCount;
			_Surfaces[i]._VertexCount = shdr[i]._VertexCount;
			
			if(_Surfaces[i]._TriangleCount)
			{
				if(!fp.Load(shdr[i]._VertexAttribute, buf))return false;
				if(buf.GetSize() != _Surfaces[i].GetVertexAttributeSize()*_Surfaces[i]._VertexCount)return false;
				if(_Surfaces[i]._VertexAttrib.Create()){ _Surfaces[i]._VertexAttrib.SetSize(buf.GetSize(), buf); }
				else{ _LOG_POS; }
				
				if(_Surfaces[i]._DataFlag & Surface::DF_POSITION_3F)
				{
					rt::Vec3d	c;
					rt::Zero(c);
					rt::Vec3f*	v = (rt::Vec3f*)buf.Begin();
					for(UINT j=0;j<_Surfaces[i]._VertexCount;j++)c += v[j];
					c /= _Surfaces[i]._VertexCount;
					_Surfaces[i]._Center = c;
				}

				if(!fp.Load(shdr[i]._Triangles, buf))return false;

				UINT	ic = _Surfaces[i]._TriangleCount*3;
				UINT	vc = _Surfaces[i]._VertexCount;

				if(_Surfaces[i]._DataFlag & Surface::DF_INDEX_1U)
				{	if(	buf.GetSize() != sizeof(UINT)*3*_Surfaces[i]._TriangleCount)return false;
					// verify index
					UINT* p = (UINT*)buf.Begin();
					for(UINT q=0;q<ic;q++)
					{	ASSERT(p[q] < vc);
					}
				}
				else if(_Surfaces[i]._DataFlag & Surface::DF_INDEX_1W)
				{	if(buf.GetSize() != sizeof(WORD)*3*_Surfaces[i]._TriangleCount)return false;	
					WORD* p = (WORD*)buf.Begin();
					for(UINT q=0;q<ic;q++)
					{	ASSERT(p[q] < vc);
					}
				}
				else return false;
				if(_Surfaces[i]._Triangles.Create()){ _Surfaces[i]._Triangles.SetSize(buf.GetSize(), buf); }
				else{ _LOG_POS; }

			}
		}

		fp.Load(Header::FILENAME_MATERIALCHUNK, mhdr);
		if(hdr.MaterialCount == mhdr.GetSize())
		{
			_Materials.SetSize(hdr.MaterialCount);
			for(UINT i=0;i<_Materials.GetSize();i++)
			{
				Material& m = _Materials[i];
				if(mhdr[i]._MaterialType == MATERIAL_TYPE_PHONG)
				{	
					if(fp.Load(mhdr[i]._MaterialData, &m.data_phong()))
					{
						_Materials[i]._Name = mhdr[i]._MaterialName;
						_BindPhongShader(_Materials[i]);
						_LOG(i<<") "<<mhdr[i]._MaterialName<<" SH="<<m._MaterialShaderIndex);
					}
				}
				else
				{	// more types
				}

				_UpdateRenderingOrder();
			}
		}

		// bind material index
		for(UINT i=0;i<_Surfaces.GetSize();i++)
		{
			_Surfaces[i]._MaterialIndex = -1;
			rt::String_Ref n = _Surfaces[i]._MaterialName;
			for(UINT m=0;m<_Materials.GetSize();m++)
				if(n == _Materials[m]._Name)
				{	_Surfaces[i]._MaterialIndex = m;
					break;
				}
		}

		_UpdateRenderingOrder();
	}

	return true;
}

void Object::_UpdateRenderingOrder()
{
	for(UINT i=0;i<_Surfaces.GetSize();i++)
	{
		int mi = _Surfaces[i]._MaterialIndex;
		if(mi < (int)_Materials.GetSize() && mi>=0)
		{
			_Surfaces[i]._RenderOrder = _Materials[mi].GetRenderOrder();
		}
		else
		{	_Surfaces[i]._RenderOrder = rt::TypeTraits<UINT>::MaxVal();
		}
	}

	_Surfaces.Sort();
}

void Object::_BindPhongShader(Material& m)
{
	MaterialPhong& phong = m.data_phong();

	m._MaterialConfig = MATERIAL_TYPE_PHONG<<MATERIAL_TYPE_SHIFT;

	if(phong.Ambient.L2NormSqr() > 0)m._MaterialConfig |= MATERIAL_PHONG_USE_KA;
	if(phong.Diffuse.L2NormSqr() > 0)m._MaterialConfig |= MATERIAL_PHONG_USE_KD;
	if(phong.Specular.L2NormSqr() > 0)m._MaterialConfig |= MATERIAL_PHONG_USE_KS;
	if(phong.Opacity < 0.999999999f)m._MaterialConfig |= MATERIAL_USE_ALPHA;

	m._MaterialShaderIndex = _GetShaderIndex(m._MaterialConfig);
}

bool Object::Prepare()	// call with RC used
{
	return false;
}

void Material::Use(ShaderProgramBase& prog)
{
	prog.UnsetAllTextures();

	if(((_MaterialConfig&MATERIAL_TYPE_MASK)>>MATERIAL_TYPE_SHIFT) == MATERIAL_TYPE_PHONG)
	{	
		prog.SetUniformArray1("_MaterialData_Phong", (float*)_MaterialData, 11);
	}
	else
	{	ASSERT(0);
	}
}

UINT  Material::GetRenderOrder() const
{
	return _MaterialConfig;
}

void Object::Render(const gl::Camera& cam)
{
	MaterialShader* shader = NULL;
	int				shader_id = -1;

	glDisable(GL_BLEND);

	for(UINT i=0;i<_Surfaces.GetSize();i++)
	{
		Surface& s = _Surfaces[i];
		if(s._TriangleCount && s._MaterialIndex>=0 && (s._DataFlag&Surface::DF_POSITION_3F) && s._VertexAttrib.GetSize() && s._Triangles.GetSize())
		{
			Material& mtl = _Materials[s._MaterialIndex];
			if(mtl._MaterialShaderIndex>=0 && mtl._MaterialShaderIndex<(int)_Shaders.GetSize())
			{
				if(shader_id != mtl._MaterialShaderIndex)
				{
					shader = &_Shaders[mtl._MaterialShaderIndex];
					shader->_Shader.Use();
					cam.ApplyTo(shader->_Shader,"_MVP", "_NormTrans");
					shader_id = mtl._MaterialShaderIndex;

					if(mtl._MaterialConfig&MATERIAL_USE_ALPHA)
					{	glEnable(GL_BLEND);
						glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					}
				}
				mtl.Use(shader->_Shader);

				UINT va_off = 0;
				s._VertexAttrib.Use();

				shader->_Shader.SetVertexAttributePointer("_Position", 0, 3, GL_FLOAT);
				va_off += s._VertexCount*3*sizeof(float);

				if(s._DataFlag&Surface::DF_NORMAL_3B)
				{	if(mtl._MaterialConfig&(MATERIAL_PHONG_USE_KD|MATERIAL_PHONG_USE_KS))
					{	shader->_Shader.SetVertexAttributePointer("_Normal", (LPVOID)va_off, 3, GL_UNSIGNED_BYTE);	}
					va_off += s._TriangleCount*3*sizeof(BYTE);
				}

				s._Triangles.Use();
				glDrawElements(GL_TRIANGLES, s._TriangleCount*3, (s._DataFlag&Surface::DF_INDEX_1U)?GL_UNSIGNED_INT:GL_UNSIGNED_SHORT, NULL);
			}
		}
	}
}

} // namespace gl






#if defined(PLATFORM_WIN)

namespace gl
{

struct MtlConv
{
	struct convMaterial
	{
		rt::String			_MaterialName;
		int					_ShadingModel;

		rt::Vec3f	Ambient;		// Ka
		rt::Vec3f	Diffuse;		// Kd
		rt::Vec3f	Specular;		// Ks
		float		Shininess;		// Ns
		float		Opacity;		// d, Tr

		rt::String  map_Ka;
		rt::String  map_Kd;
		rt::String  map_Ks;
		rt::String  map_Ns;
		rt::String  map_d;

		convMaterial()
		{	_ShadingModel = -1;
			rt::Zero(Ambient);	rt::Zero(Diffuse);	rt::Zero(Specular);
			Shininess = -1;
			Opacity = 2;
		}
	};
	rt::BufferEx<convMaterial*>	convMaterials;
	convMaterial* pm;

	typedef rt::hash_map<rt::String, int, rt::String::hash_compare>	t_NameMap;
	t_NameMap	_NameMap;

	MtlConv()
	{	pm = NULL;
	}
	~MtlConv()
	{	for(UINT i=0;i<convMaterials.GetSize();i++)
			_SafeDel(convMaterials[i]);
	}

	int	GetMaterialByName(const rt::String_Ref& name)
	{
		t_NameMap::const_iterator it = _NameMap.find(name);
		if(it != _NameMap.end())
			return it->second;
		else
		{	_LOG(name<<" is not found");
			return -1;
		}
	}
	
	bool ParseLine_MtlAlias(rt::String_Ref line)
	{
		rt::String_Ref f[2];
		if(line.Split(f,2,'=') == 2)
		{
			f[1] = f[1].TrimSpace();
			f[0] = f[0].TrimSpace();
			 int t = GetMaterialByName(f[1]);
			 if(t>=0)
				 _NameMap[f[0]] = t;
		}
		return true;
	}

	bool ParseLine_Mtl(rt::String_Ref line)
	{
		if(line.StartsWith("newmtl "))
		{	pm = convMaterials.push_back(new convMaterial);
			pm->_MaterialName = line.TrimLeft(7);
			_NameMap[pm->_MaterialName] = (int)convMaterials.GetSize()-1;
		}
		else if(line.StartsWith("alias "))
		{	
			rt::String_Ref f[2];
			if(line.TrimLeft(6).Split(f,2,'=') == 2)
			{
				f[1] = f[1].TrimSpace();
				f[0] = f[0].TrimSpace();
				 int t = GetMaterialByName(f[1]);
				 if(t>=0)
					 _NameMap[f[0]] = t;
			}
		}
		else
		{	if(pm == NULL)return true;
			if(line.StartsWith("Ka "))
			{	pm->Ambient.FromString(line.TrimLeft(3));	}
			else
			if(line.StartsWith("Kd "))
			{	pm->Diffuse.FromString(line.TrimLeft(3));	}
			else
			if(line.StartsWith("Ks "))
			{	pm->Specular.FromString(line.TrimLeft(3));	}
			else
			if(line.StartsWith("Ns "))
			{	line.TrimLeft(3).ToNumber(pm->Shininess);	}
			else
			if(line.StartsWith("d "))
			{	line.TrimLeft(2).ToNumber(pm->Opacity);	}
			else
			if(line.StartsWith("illum "))
			{	line.TrimLeft(6).ToNumber(pm->_ShadingModel);	}
			else
			if(line.StartsWith("map_Ka ")){	pm->map_Ka = line.TrimLeft(7);	}
			else
			if(line.StartsWith("map_Kd ")){	pm->map_Kd = line.TrimLeft(7);	}
			else
			if(line.StartsWith("map_Ks ")){	pm->map_Ks = line.TrimLeft(7);	}
			else
			if(line.StartsWith("map_Ns ")){	pm->map_Ns = line.TrimLeft(7);	}
			else
			if(line.StartsWith("map_d ")){	pm->map_d = line.TrimLeft(7);	}
		}

		return true;
	}
};

struct ObjConv
{
	typedef rt::hash_map<rt::Vec3u, int, rt::Vec3u::hash_compare> t_VertexTable;

	struct convSurface
	{
		UINT				_MaterialIndex;
		DWORD				_DataFlag;
		rt::String			_GroupName;
		rt::String			_MaterialName;

		rt::Buffer<BYTE>	_VertexAttribute;
		rt::Buffer<BYTE>	_Triangles;

		int						 vertex_count;
		t_VertexTable			 vertex;
		rt::BufferEx<rt::Vec3u>	 triangle;

		convSurface()
		{	vertex_count = 0;
			_DataFlag = 0;
		}
	};
	rt::BufferEx<convSurface*>	convSurfaces;
	convSurface* ps;
	rt::String	Filename;

	MtlConv		mtlconv;

	rt::String mtl_filename;
	rt::BufferEx<rt::Vec3f>	 pos;
	rt::BufferEx<rt::Vec3b>	 norm;
	rt::BufferEx<rt::Vec2f>	 texc;

	rt::Vec3f  bmin, bmax;

	ObjConv()
	{	
		ps = convSurfaces.push_back(new convSurface);
		bmin.Set(rt::TypeTraits<float>::MaxVal());
		bmax.Set(rt::TypeTraits<float>::MinVal());
	}
	~ObjConv()
	{	for(UINT i=0;i<convSurfaces.GetSize();i++)
			_SafeDel(convSurfaces[i]);
	}
	bool ParseLine_Obj(rt::String_Ref line)
	{
		if(line[0] == 'v')
		{
			if(line[1] == ' ')
			{	rt::Vec3f& v = pos.push_back();
				v.FromString(line.TrimLeft(2)); 
				bmin.Min(bmin, v);
				bmax.Max(bmax, v);
			}
			else
			if(line[1] == 'n')
			{	rt::Vec3f	nf(line.TrimLeft(3));
				rt::Vec3b& n = norm.push_back();
				n.x = (BYTE)((nf.x + 1)*127 + 0.5f);
				n.y = (BYTE)((nf.y + 1)*127 + 0.5f);
				n.z = (BYTE)((nf.z + 1)*127 + 0.5f);
			}
			else
			if(line[1] == 't'){ texc.push_back().FromString(line.TrimLeft(3)); }
		}
		else if(line[0] == 'f')
		{
			rt::String_Ref index = line.TrimLeft(2).TrimSpace();
			rt::String_Ref v3[3];
			if(3 == index.Split(v3, 3, ' '))
			{	
				rt::Vec3u t;
				rt::String_Ref vi[3];
				rt::Vec3u v[3];

				for(int i=0;i<3;i++)
				{
					int f = v3[i].Split(vi, 3, '/');
					vi[0].ToNumber(v[i].x);
					vi[1].ToNumber(v[i].y);
					vi[2].ToNumber(v[i].z);

					v[i] -= 1;	// obj index is 1-based
				}

				int df = (vi[0].IsEmpty()?0:Surface::DF_POSITION_3F) |
						 (vi[2].IsEmpty()?0:Surface::DF_NORMAL_3B) |
						 (vi[1].IsEmpty()?0:Surface::DF_TEXCOORD_2F);

				if(	df != ps->_DataFlag)
				{	StartNextSurface(ps->_MaterialName, df);
				}

				for(int i=0;i<3;i++)
				{
					t_VertexTable::const_iterator it = ps->vertex.find(v[i]);
					if(it == ps->vertex.end())
					{	t[i] = ps->vertex_count;
						ps->vertex[v[i]] = ps->vertex_count;
						ps->vertex_count++;
					}
					else
					{	t[i] = it->second;
						if(it->second >= ps->vertex_count)_LOG_POS;
					}
				}

				ps->triangle.push_back(t);
			}
			else return false;
		}
		else if(line[0] == 'g')
		{	//CompleteSurface();
			ps->_GroupName = line.TrimLeft(2);
		}
		else if(line.StartsWith(rt::String_Ref("usemtl ",7)))
		{	
			StartNextSurface(line.TrimLeft(7), ps->_DataFlag);
		}
		else if(line.StartsWith(rt::String_Ref("mtllib ",7)))
		{
			mtl_filename = line.TrimLeft(7);

			os::FileReadLine	mtl_file;
			if(mtl_file.Open(rt::String_Ref(Filename).GetDirectoryName() + mtl_filename))
			{
				while(mtl_file.GetNextLine(line))
				{
					if(!mtlconv.ParseLine_Mtl(line))
						return false;
				}
			}
		}

		return true;
	}
	void StartNextSurface(const rt::String_Ref& mtl_name, int dataflag)
	{
		if(ps->triangle.GetSize())
		{
			_LOG(ps->_DataFlag<<'\t'<<mtl_name<<": T="<<ps->triangle.GetSize()<<" V="<<ps->vertex_count);

			int mindex = mtlconv.GetMaterialByName(mtl_name);

			for(UINT i=0;i<convSurfaces.GetSize();i++)
				if(	convSurfaces[i]->_MaterialIndex == mindex &&
					convSurfaces[i]->_DataFlag == dataflag
				)
				{	ps = convSurfaces[i];
					return;
				}

			ps = convSurfaces.push_back(new convSurface);
			ps->_GroupName = convSurfaces[convSurfaces.GetSize()-2]->_GroupName;
			ps->_MaterialIndex = mindex;
			if(mindex>=0)
				ps->_MaterialName = mtlconv.convMaterials[mindex]->_MaterialName;
		}

		ps->_DataFlag = dataflag;
	}
	template<typename T>
	void _CopyVertexData(rt::Vec3f* ppos, rt::Vec2f* ptexc, rt::Vec3b* pnorm, T* tri, const convSurface* cs)
	{	
		for(UINT i=0; i<cs->triangle.GetSize(); i++)
			tri[i] = cs->triangle[i];
		
		t_VertexTable::const_iterator it = cs->vertex.begin();
		for(; it != cs->vertex.end(); it++)
		{	
			const rt::Vec3u& v = it->first;
			int i = it->second;

			if(ppos)ppos[i] = pos[v.x];
			if(ptexc)ptexc[i] = texc[v.y];
			if(pnorm)pnorm[i] = norm[v.z];
		}
	}
};

bool Object::ReloadPhongMaterial(LPCSTR filename, LPCSTR mtl_alias)
{
	MtlConv	mi;
	os::FileReadLine	mtl_file;
	rt::String_Ref		line;
	if(mtl_file.Open(filename))
	{
		while(mtl_file.GetNextLine(line))
		{
			if(!mi.ParseLine_Mtl(line))
				return false;
		}
	}
	line.Empty();
	if(mtl_file.Open(mtl_alias))
	{
		while(mtl_file.GetNextLine(line))
		{
			if(!mi.ParseLine_MtlAlias(line))
				return false;
		}
	}
	
	_Materials.SetSize(mi.convMaterials.GetSize());

	for(UINT i=0;i<_Materials.GetSize();i++)
	{
		MtlConv::convMaterial& cm = *mi.convMaterials[i];
		Material& m = _Materials[i];
		m._Name = cm._MaterialName;

		MaterialPhong& pg = m.data_phong();
		pg.Ambient = cm.Ambient;
		pg.Diffuse = cm.Diffuse;
		pg.Specular = cm.Specular;
		pg.Shininess = cm.Shininess;
		pg.Opacity = cm.Opacity;

		_BindPhongShader(_Materials[i]);

		pg.AmbientMapIndex = INFINITE;
		pg.DiffuseMapIndex = INFINITE;
		pg.SpecularMapIndex = INFINITE;
		pg.ShininessMapIndex = INFINITE;
		pg.OpacityMapIndex = INFINITE;
	}

	for(UINT i=0;i<_Surfaces.GetSize();i++)
		_Surfaces[i]._MaterialIndex = mi.GetMaterialByName(_Surfaces[i]._MaterialName);

	_UpdateRenderingOrder();
	return true;
}

bool Object::ConvertObjMtlPair(LPCSTR filename, LPCSTR save_as, bool normalize_size)
{
	os::FilePacked_Writer		out;
	if(!out.Open(save_as))return false;

	os::FileReadLine	file;
	if(!file.Open(filename))return false;

	ObjConv	oi;
	oi.Filename = filename;

	rt::String_Ref line;
	while(file.GetNextLine(line))
	{
		if(!oi.ParseLine_Obj(line))
			return false;
	}

	if(normalize_size)
	{
		rt::Vec3f center = oi.bmax;
		center += oi.bmin;
		center /= 2;
		oi.bmax -= oi.bmin;
		float scale = rt::max(oi.bmax.x, rt::max(oi.bmax.y, oi.bmax.z));

		for(UINT i=0;i<oi.pos.GetSize();i++)
		{	rt::Vec3f& v = oi.pos[i];
			v -= center;
			v /= scale;
		}
	}

	ULONGLONG filen = 1000;
	rt::Buffer<SurfaceChunk>	shdr;
	shdr.SetSize(oi.convSurfaces.GetSize());
		
	for(UINT i=0;i<oi.convSurfaces.GetSize();i++)
	{
		SurfaceChunk& s = shdr[i];
		ObjConv::convSurface& cs = *oi.convSurfaces[i];
		s._DataFlag = cs._DataFlag;
		rt::Zero(s._SurfaceName);
		memcpy(s._SurfaceName, cs._GroupName, rt::min((SIZE_T)127, cs._GroupName.GetLength()));
		rt::Zero(s._MaterialName);
		memcpy(s._MaterialName, cs._MaterialName, rt::min((SIZE_T)127, cs._MaterialName.GetLength()));

		rt::Buffer<BYTE>	_VertexAttribute;
		rt::Buffer<BYTE>	_Triangles;

		int size = 0;

		if(s._DataFlag & Surface::DF_POSITION_3F){ size += cs.vertex_count * sizeof(float)*3; }
		if(s._DataFlag & Surface::DF_NORMAL_3B){ size += cs.vertex_count * sizeof(BYTE)*3; }
		if(s._DataFlag & Surface::DF_TEXCOORD_2F){ size += cs.vertex_count * sizeof(float)*2; }

		_VertexAttribute.SetSize(size);
		int offset = 0;
		rt::Vec3f* ppos = NULL;
		rt::Vec3b* pnorm = NULL;
		rt::Vec2f* ptexc = NULL;

		if(s._DataFlag & Surface::DF_POSITION_3F){ ppos = (rt::Vec3f*)&_VertexAttribute[offset]; offset += cs.vertex_count * sizeof(float)*3; }
		if(s._DataFlag & Surface::DF_NORMAL_3B){ pnorm = (rt::Vec3b*)&_VertexAttribute[offset]; offset += cs.vertex_count * sizeof(BYTE)*3; }
		if(s._DataFlag & Surface::DF_TEXCOORD_2F){ ptexc = (rt::Vec2f*)&_VertexAttribute[offset]; offset += cs.vertex_count * sizeof(float)*2; }

		if(cs.vertex_count > 0xffff)
		{	_Triangles.SetSize(sizeof(UINT)*cs.triangle.GetSize()*3);
			s._DataFlag |= Surface::DF_INDEX_1U;
			oi._CopyVertexData<rt::Vec3u>(ppos, ptexc, pnorm, (rt::Vec3u*)_Triangles.Begin(), &cs);
		}
		else
		{	_Triangles.SetSize(sizeof(WORD)*cs.triangle.GetSize()*3);
			s._DataFlag |= Surface::DF_INDEX_1W;
			oi._CopyVertexData<rt::Vec3w>(ppos, ptexc, pnorm, (rt::Vec3w*)_Triangles.Begin(), &cs);
		}

		s._VertexAttribute = filen;
		s._VertexCount = cs.vertex_count;
		out.Write(filen++, _VertexAttribute, (UINT)_VertexAttribute.GetSize());
		s._Triangles = filen;
		s._TriangleCount = (UINT)cs.triangle.GetSize();
		out.Write(filen++, _Triangles, (UINT)_Triangles.GetSize());

		
	}

	out.Write(Header::FILENAME_SURFACECHUNK, shdr, (UINT)shdr.GetSize()*sizeof(SurfaceChunk));

	filen+=1000;
	rt::Buffer<MaterialChunk>	mhdr;
	mhdr.SetSize(oi.mtlconv.convMaterials.GetSize());

	for(UINT i=0;i<oi.mtlconv.convMaterials.GetSize();i++)
	{
		MtlConv::convMaterial& cm = *oi.mtlconv.convMaterials[i];
		MaterialChunk& m = mhdr[i];
		rt::Zero(m._MaterialName);
		memcpy(m._MaterialName, cm._MaterialName, rt::min((SIZE_T)127, cm._MaterialName.GetLength()));

		m._MaterialType = MATERIAL_TYPE_PHONG;
		m._MaterialConfig = cm._ShadingModel;
		m._MaterialData = filen;

		MaterialPhong pg;
		pg.Ambient = cm.Ambient;
		pg.Diffuse = cm.Diffuse;
		pg.Opacity = cm.Opacity;
		pg.Shininess = cm.Shininess;
		pg.Specular = cm.Specular;

		pg.AmbientMapIndex = INFINITE;
		pg.DiffuseMapIndex = INFINITE;
		pg.SpecularMapIndex = INFINITE;
		pg.ShininessMapIndex = INFINITE;
		pg.OpacityMapIndex = INFINITE;

		out.Write(filen++, &pg, sizeof(pg));
	}

	out.Write(Header::FILENAME_MATERIALCHUNK, mhdr, (UINT)mhdr.GetSize()*sizeof(MaterialChunk));

	Header hdr;
	hdr.Magic = Header::Magic_xScene;
	hdr.SurfaceCount = (UINT)oi.convSurfaces.GetSize();
	hdr.MaterialCount = (UINT)oi.mtlconv.convMaterials.GetSize();

	out.Write(Header::FILENAME_HEADER, &hdr, sizeof(hdr));

	return true;
}

} // namespace gl
#endif


