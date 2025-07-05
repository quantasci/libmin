//--------------------------------------------------------------------------------
// NVIDIA(R) GVDB VOXELS
// Copyright 2017, NVIDIA Corporation. 
//
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
//    in the documentation and/or  other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived 
//    from this software without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Version 1.0: Rama Hoetzlein, 5/1/2017
//----------------------------------------------------------------------------------


#include "vec.h"

#ifndef DEF_PIVOTX
	#define DEF_PIVOTX
	
	class HELPAPI PivotX {
	public:
		PivotX()	{ from_pos.Set(0,0,0); to_pos.Set(0,0,0); ang_euler.Set(0,0,0); scale.Set(1,1,1); trans.Identity(); }
		PivotX( Vec3F& f, Vec3F& t, Vec3F& s, Vec3F& a) { from_pos=f; to_pos=t; scale=s; ang_euler=a; }

		void setPivot ( float x, float y, float z, float rx, float ry, float rz );
		void setPivot ( Vec3F& pos, Vec3F& ang ) { from_pos = pos; ang_euler = ang; }
		void setPivot ( PivotX  piv )	{ from_pos = piv.from_pos; to_pos = piv.to_pos; ang_euler = piv.ang_euler; updateTform(); }		
		void setPivot ( PivotX& piv )	{ from_pos = piv.from_pos; to_pos = piv.to_pos; ang_euler = piv.ang_euler; updateTform(); }

		void setIdentity ()		{ from_pos.Set(0,0,0); to_pos.Set(0,0,0); ang_euler.Set(0,0,0); scale.Set(1,1,1); trans.Identity(); }

		void setAng ( float rx, float ry, float rz )	{ ang_euler.Set(rx,ry,rz);	updateTform(); }
		void setAng ( Vec3F& a )					{ ang_euler = a;			updateTform(); }

		void setPos ( float x, float y, float z )		{ from_pos.Set(x,y,z);		updateTform(); }
		void setPos ( Vec3F& p )					{ from_pos = p;				updateTform(); }

		void setToPos ( float x, float y, float z )		{ to_pos.Set(x,y,z);		updateTform(); }
		
		void updateTform ();
		void setTform ( Matrix4F& t )		{ trans = t; }
		inline Matrix4F& getTform ()		{ return trans; }
		inline float* getTformData ()		{ return trans.GetDataF(); }

		// Pivot		
		PivotX getPivot ()	{ return PivotX(from_pos, to_pos, scale, ang_euler); }
		Vec3F& getPos ()			{ return from_pos; }
		Vec3F& getToPos ()			{ return to_pos; }
		Vec3F& getAng ()			{ return ang_euler; }
		Vec3F getDir ()			{ 
			return to_pos - from_pos; 
		}
		void getPreciseEye (Vec3F& hi, Vec3F& lo)
		{
			//-- should use a double precision camera position as input
			// see: https://prideout.net/emulating-double-precision	
			hi = from_pos;
			lo.Set(0,0,0);
		}	
		Vec3F	from_pos;
		Vec3F	to_pos;
		Vec3F	scale;
		Vec3F	ang_euler;
		Matrix4F	trans;
		
		//Quatern	ang_quat;
		//Quatern	dang_quat;
	};

#endif

#ifndef DEF_CAMERA_3D
	#define	DEF_CAMERA_3D
	
	#define DEG_TO_RAD			(3.141592/180.0)

	class HELPAPI Camera3D : public PivotX {
	public:
		enum eProjection {
			Perspective = 0,
			Parallel = 1
		};
		Camera3D ();
		void Copy ( Camera3D& op );

		void draw_gl();

		// Camera motion
		void LookAt ();
		void SetOrientation ( Vec3F angs, Vec3F pos );		// roll, pitch, yaw
		void SetMatrices (const float* view_mtx, const float* proj_mtx, Vec3F model_pos );
		void SetOrbit  ( float ax, float ay, float az, Vec3F tp, float dist, float dolly );		
		void SetOrbit  ( Vec3F angs, Vec3F tp, float dist, float dolly );

		// Camera settings		
		void setPos ( float x, float y, float z )		{ from_pos.Set(x,y,z);		updateView(); }
		void setToPos ( float x, float y, float z )		{ to_pos.Set(x,y,z);		updateView(); }
		void setAspect ( float asp )					{ mAspect = asp;			updateProj(); }
		void setFov (float fov)							{ mFov = fov;				updateProj(); }
		void setNearFar (float n, float f )				{ mNear = n; mFar = f;		updateProj(); }
		void setProjection (eProjection proj_type)		{ mProjType = proj_type;	updateProj(); }
		void setTile ( float x1, float y1, float x2, float y2 )		{ mTile.Set ( x1, y1, x2, y2 );		updateProj(); }

		void setDOF ( Vec3F dof )					{ mDOF = dof; }
		void setDolly ( float d )						{ mDolly = d; }
		void setDist ( float d )						{ mOrbitDist = d; }		
		void setSize ( float w, float h )				{ mXres=w; mYres=h; }
		Vec3F getSize ()									{ return Vec3F(mXres, mYres, 1); }
		
		void setModelMatrix ( float* mtx );
		void setViewMatrix ( float* mtx );
		void setProjMatrix ( float* mtx );	

		void setAngles ( float ax, float ay, float az );		// ONLY pitch, yaw
		void setDirection ( Vec3F from, Vec3F to, float roll=0.0 );
		void moveOrbit ( float ax, float ay, float az, float dist );		
		void moveToPos ( float tx, float ty, float tz );		
		void moveRelative ( float dx, float dy, float dz );		

		// Frustum testing
		bool pointInFrustum ( float x, float y, float z );
		bool boxInFrustum ( Vec3F bmin, Vec3F bmax);
		float calculateLOD ( Vec3F pnt, float minlod, float maxlod, float maxdist );

		// Utility functions
		void updateProj ();				// rebuild proj_matrix, tileproj
		void updateView ();				// rebuild view_matrix, invview
		void updateFrustum ();			// updates frustum planes
		void updateAll ();
		void getBounds ( Vec2F cmin, Vec2F cmax, float dst, Vec3F& min, Vec3F& max);
		void getBounds ( float dst, Vec3F& min, Vec3F& max );
		Vec3F inverseRay ( float x, float y, float xres, float yres, float z=1.0 );
		Vec3F inverseRayProj ( float x, float y, float z );
		Vec4F project ( Vec3F& p );
		Vec4F project ( Vec3F& p, Matrix4F& vm );		// Project point - override view matrix

		void getVectors ( Vec3F& dir, Vec3F& up, Vec3F& side )	{ dir = dir_vec; up = up_vec; side = side_vec; }		
		Vec3F getNearFar()			{ return Vec3F(mNear, mFar, mFov); }
		float getNear ()				{ return mNear; }
		float getFar ()					{ return mFar; }
		float getFov ()					{ return mFov; }
		float getDolly()				{ return mDolly; }	
		float getOrbitDist()			{ return mOrbitDist; }
		Vec3F& getUpDir ()			{ return up_dir; }
		Vec4F& getTile ()			{ return mTile; }
		Vec3F& getDOF ()			{ return mDOF; }
		float getAspect ()				{ return mAspect; }

		// new interface
		Matrix4F& getViewMatrix()		{ return view_matrix; }
		Matrix4F& getProjMatrix ()		{ return tileproj_matrix; }	
		Matrix4F& getFullProjMatrix ()	{ return proj_matrix; }
		Matrix4F& getModelMatrix()		{ return model_matrix; }		
		Matrix4F  getViewInv()			{ Matrix4F m; return m.makeOrthogonalInverse (view_matrix); }
		Matrix4F  getRotateMtx()		{ Matrix4F m; return m.makeRotationMtx (view_matrix); }
		Matrix4F  getRotateInv()		{ Matrix4F m; return m.makeRotationInv (view_matrix); }		
		Matrix4F  getProjInv()			{ Matrix4F m; return m.InverseProj ( tileproj_matrix.GetDataF() ); }		
		Matrix4F  getViewProjInv();
		Matrix4F  getUVWMatrix();
		
		Vec3F getU ();
		Vec3F getV ();
		Vec3F getW ();
		float getDu ();
		float getDv ();


	public:
		eProjection		mProjType;								// Projection type

		// Camera Parameters									// NOTE: Pivot maintains camera from and orientation
		float			mDolly;									// Camera to distance
		float			mOrbitDist;
		float			mFov, mAspect;							// Camera field-of-view
		float			mNear, mFar;							// Camera frustum planes
		float			mXres, mYres;
		Vec3F		dir_vec, side_vec, up_vec;				// Camera aux vectors (W, V, and U)
		Vec3F		up_dir;
		Vec4F		mTile;
		Vec3F		mDOF;
		
		// Transform Matricies		
		Matrix4F		view_matrix;							// V matrix	(rotation + translation)
		Matrix4F		proj_matrix;							// P matrix		
		Matrix4F		tileproj_matrix;						// tiled projection matrix
		Matrix4F		model_matrix;		
		float			frustum[6][4];							// frustum plane equations

		bool			mOps[8];
		int				mWire;
		
		Vec3F		origRayWorld;
		Vec4F		tlRayWorld;
    	Vec4F		trRayWorld;
    	Vec4F		blRayWorld;
    	Vec4F		brRayWorld;
	};

	typedef Camera3D		Light;

#endif
