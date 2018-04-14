#ifndef STATE_IMU_H
#define STATE_IMU_H

#include "game_state.h"
#include "scene_object.h"

#include "components/transform.h"
#include "components/camera.h"
#include "components/solid_mesh.h"
#include "components/material.h"
#include "components/light_omni.h"

#include <serial/serial.h>

#include <mutex>

#include "lib/imu/attitude_estimator.h"

DWORD WINAPI ReadThread(LPVOID lpParam);

class IMUDevice
{
public:
    IMUDevice()
    : quat(0.0f, 0.0f, 0.0f, 1.0f),
    magnet_max(1.0f, 1.0f, 1.0f),
    magnet_max_prev(0.0f, 0.0f, 0.0f),
    magnet_min(-1.0f, -1.0f, -1.0f),
    magnet_min_prev(0.0f, 0.0f, 0.0f),
    magnet_mid(0.0f, 0.0f, 0.0f)
    {
        ListPorts();
        
        DWORD threadId = 0;
        hThread = CreateThread(0, 0, &ReadThread, (void*)this, 0, &threadId);
    }
    ~IMUDevice()
    {
        delete port;
    }
    
    void Init()
    {
        try
        {
            port = new serial::Serial("COM4", 115200, serial::Timeout::simpleTimeout(1000));
        }
        catch(serial::PortNotOpenedException& ex)
        {
            std::cout << ex.what() << std::endl;
        }
        catch(serial::IOException& ex)
        {
            std::cout << ex.what() << std::endl;
        }
        catch(std::invalid_argument& ex)
        {
            std::cout << ex.what() << std::endl;
        }
    }
    
    void ReadFunc()
    {
        stateestimation::AttitudeEstimator est;
        est.setMagCalib(0.68, -1.32, 0.0);
        est.setPIGains(2.2, 2.65, 10, 1.25);
        while(port->isOpen())
        {
            float gx = 0, gy = 0, gz = 0;
            float ax = 0, ay = 0, az = 0;
            float mx = 0, my = 0, mz = 0;
            float q0 = 0, q1 = 0, q2 = 0, q3 = 0;
            int16_t data[9];
            float qt[4];
            port->readline(65536, "Package");
            port->read((uint8_t*)data, sizeof(int16_t) * 9);
            port->read((uint8_t*)qt, sizeof(float) * 4);
            gx = data[0]; gy = data[1]; gz = data[2];
            ax = data[3]; ay = data[4]; az = data[5];
            mx = data[6]; my = data[7]; mz = data[8];
            q0 = qt[0]; q1 = qt[1]; q2 = qt[2]; q3 = qt[3];
            
            gx = (250.0 * (gx / 32767.0)) * 0.0174533;
            gy = (250.0 * (gy / 32767.0)) * 0.0174533;
            gz = (250.0 * (gz / 32767.0)) * 0.0174533;
            
            ax = (2.0 * (ax / 32767.0));
            ay = (2.0 * (ay / 32767.0));
            az = (2.0 * (az / 32767.0));
            
            magnet_max_prev = magnet_max;
            magnet_min_prev = magnet_min;
            /*
            if(abs(mx) < 1000)
            {
                if(mx > magnet_max.x) magnet_max.x = mx;
                if(mx < magnet_min.x) magnet_min.x = mx;
            }
            if(abs(my) < 1000)
            {
                if(my > magnet_max.y) magnet_max.y = my;
                if(my < magnet_min.y) magnet_min.y = my;
            }
            if(abs(mz) < 1000)
            {
                if(mz > magnet_max.z) magnet_max.z = mz;
                if(mz < magnet_min.z) magnet_min.z = mz;
            }
            
            magnet_max = (magnet_max + magnet_max_prev) * 0.5f;
            magnet_min = (magnet_min + magnet_min_prev) * 0.5f;
            */
            
            magnet_max.x = 226.587f;
            magnet_max.y = 335.595f;
            magnet_max.z = 265.873f;
            magnet_min.x = -271.742f;
            magnet_min.y = -108.647f;
            magnet_min.z = -345.472f;
            
            float mx_half = (magnet_max.x + magnet_min.x) * 0.5f;
            float my_half = (magnet_max.z + magnet_min.y) * 0.5f;
            float mz_half = (magnet_max.y + magnet_min.z) * 0.5f;
            
            float avg_range = (magnet_max.x - magnet_min.x) + (magnet_max.y - magnet_min.y) + (magnet_max.z - magnet_min.z);
            avg_range = avg_range / 3.0f;
            float avg_mod_x = avg_range / (magnet_max.x - magnet_min.x);
            float avg_mod_y = avg_range / (magnet_max.y - magnet_min.y);
            float avg_mod_z = avg_range / (magnet_max.z - magnet_min.z);
            
            mx = (mx - mx_half) / avg_mod_x;
            my = (my - my_half) / avg_mod_y;
            mz = (mz - mz_half) / avg_mod_z;
            /*
            std::cout << "\n";
            std::cout << "MAX_X: " << magnet_max.x << "\n";
            std::cout << "MIN_X: " << magnet_min.x << "\n";
            std::cout << "X: " << mx << "\n";
            std::cout << "MAX_Y: " << magnet_max.y << "\n";
            std::cout << "MIN_Y: " << magnet_min.y << "\n";
            std::cout << "Y: " << my << "\n";
            std::cout << "MAX_Z: " << magnet_max.z << "\n";
            std::cout << "MIN_Z: " << magnet_min.z << "\n";
            std::cout << "Z: " << mz << "\n";
            */
            //std::cout << mx << "\t" << my << "\t" << mz << "\n";
            
            sync_quat.lock();
            
            gyro.x = gx;
            gyro.y = gy;
            gyro.z = gz;
            
            accel.x = ax;
            accel.y = ay;
            accel.z = az;
            
            magnet.x = mx;
            magnet.y = my;
            magnet.z = mz;
            
            quat_am = _AccelMagnetFuse(accel, magnet);
            
            
            
            gfxm::vec3 angle = gfxm::vec3(1.0f, 0.0f, 0.0f);
            gfxm::quat qx = gfxm::angle_axis(gyro.x, angle);
            angle = qx * gfxm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            gfxm::quat qy = gfxm::angle_axis(gyro.y, angle);
            angle = qy * qx * gfxm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            gfxm::quat qz = gfxm::angle_axis(gyro.z, angle);
            quat_gyro = qz * qy * qx;
            
            quat.x = q1;
            quat.y = q2;
            quat.z = q3;
            quat.w = q0;
            sync_quat.unlock();
        }
    }
    
    void ListPorts()
    {
        std::vector<serial::PortInfo> devices = serial::list_ports();
        for(unsigned i = 0; i < devices.size(); ++i)
        {
            serial::PortInfo& device = devices[i];
            
            std::cout << "PORT: " << device.port << " DESC: " << device.description << " ID: " << device.hardware_id << std::endl;
        }
    }
    
    gfxm::vec3 GetGyro()
    {
        sync_quat.lock();
        gfxm::vec3 g = gyro;
        sync_quat.unlock();
        return g;
    }
    
    gfxm::vec3 GetAccel()
    {
        sync_quat.lock();
        gfxm::vec3 a = accel;
        sync_quat.unlock();
        return a;
    }
    
    gfxm::vec3 GetMagnet()
    {
        sync_quat.lock();
        gfxm::vec3 m = magnet;
        sync_quat.unlock();
        return m;
    }
    
    gfxm::quat GetQuat()
    {
        sync_quat.lock();
        gfxm::quat q = quat;
        sync_quat.unlock();
        return q;
    }
    
    gfxm::quat GetQuatAM()
    {
        sync_quat.lock();
        gfxm::quat q = quat_am;
        sync_quat.unlock();
        return q;
    }
    
    gfxm::quat GetQuatGyro()
    {
        sync_quat.lock();
        gfxm::quat q = quat_gyro;
        sync_quat.unlock();
        return q;
    }
    
private:
    gfxm::quat _AccelMagnetFuse(gfxm::vec3& accel, gfxm::vec3& magnet)
    {
        gfxm::quat q;
        
        gfxm::vec3 a = gfxm::normalize(gfxm::vec3(accel.x, accel.z, -accel.y));
        gfxm::vec3 m = gfxm::normalize(gfxm::vec3(-magnet.x, magnet.z, magnet.y));
        
        gfxm::vec3 ew = gfxm::cross(a, -m);
        gfxm::vec3 ns = gfxm::cross(ew, a);
        
        gfxm::mat3 mat;
        mat[0] = gfxm::normalize(ew);
        mat[1] = gfxm::normalize(a);
        mat[2] = gfxm::normalize(ns);
        
        //gfxm::quat orientation = gfxm::angle_axis(3.14f, gfxm::vec3(0.0f, 1.0f, 0.0f));
        
        q = gfxm::to_quat(mat);// * orientation;
        
        return q;
    }

    HANDLE hThread;
    serial::Serial* port;
    
    gfxm::vec3 magnet_max;
    gfxm::vec3 magnet_max_prev;
    gfxm::vec3 magnet_min;
    gfxm::vec3 magnet_min_prev;
    gfxm::vec3 magnet_mid;
    
    gfxm::vec3 gyro;
    std::mutex sync_gyro;
    
    std::mutex sync_quat;
    gfxm::quat quat;
    gfxm::quat quat_am;
    gfxm::quat quat_gyro;
    gfxm::vec3 accel;
    gfxm::vec3 magnet;
};

inline DWORD WINAPI ReadThread(LPVOID lpParam)
{
    IMUDevice* imu = (IMUDevice*)lpParam;
    imu->Init();
    imu->ReadFunc();
    return 0;
}

class IMUVisual : public GameState
{
public:
    virtual void OnInit()
    {        
        renderer = scene.GetComponent<Renderer>();
        renderer->Init(GFXDevice());
        renderer->AmbientColor(0.1f, 0.1f, 0.1f);
        renderer->RimColor(0.4f, 0.4f, 0.8f);
        
        camera = scene.CreateObject()->GetComponent<Camera>();
        camera->Perspective(1.6f, 16.0f/9.0f, 0.01f, 1000.0f);
        camera->GetComponent<Transform>()->Translate(0.0f, 1.5f, 5.0f);
        
        object = scene.CreateObject()->GetComponent<Transform>();
        object->GetComponent<Model>()->SetMesh("miku2");
        object->GetComponent<Model>()->SetMaterial("material");
        
        //object = camera->GetComponent<Transform>();
        
        LightDirect* l = scene.CreateObject()->GetComponent<LightDirect>();
        l->Color(0.4f, 0.3f, 0.2f);
        l->Direction(0.0f, -1.0f, 0.0f);
        
        SceneObject* o = scene.CreateObject();
        o->Name("ORIENT");
        accel = o->GetComponent<Transform>();
        accel->Translate(-4.5, 0.0, 0.0);
        o = scene.CreateObject();
        o->Name("ORIENT");
        magnet = o->GetComponent<Transform>();
        magnet->Translate(4.5, 0.0, 0.0);
    }
    
    gfxm::vec3 AccelMagnetToAngle(gfxm::vec3& accel, gfxm::vec3& magnet)
    {
        gfxm::vec3 angles;
        
        float pitch = asin(-accel.y / accel.length());
        float roll = asin(accel.x / accel.length());
        float y = (-magnet.x) * cos(roll) + magnet.z * sin(roll);
        float x = magnet.x * sin(pitch) * sin(roll) + magnet.y * cos(pitch) + magnet.z * sin(pitch) * cos(roll);
        float azimuth = atan2(y, x);
        
        angles = gfxm::vec3(azimuth, pitch, roll);
        
        return angles;
    }
    
    gfxm::quat AccelToQuat(gfxm::vec3& accel, gfxm::vec3& back)
    {
        gfxm::vec3 a;
        a.x = -accel.x;
        a.y = accel.z;
        a.z = accel.y;
        
        gfxm::quat q;
        
        gfxm::vec3 right = gfxm::cross(a, back);
        gfxm::vec3 ns = gfxm::cross(right, a);
        
        gfxm::mat3 mat;
        mat[0] = gfxm::normalize(right);
        mat[1] = gfxm::normalize(a);
        mat[2] = gfxm::normalize(ns);
        
        //gfxm::quat orientation = gfxm::angle_axis(3.14f, gfxm::vec3(0.0f, 1.0f, 0.0f));
        
        q = gfxm::to_quat(mat);// * orientation;
        
        return q;
    }
    
    virtual void OnUpdate()
    {
        gfxm::vec3 g = imu.GetGyro();
        g.x = (int)(g.x * 10) * 0.1f;
        g.y = (int)(g.y * 10) * 0.1f;
        g.z = (int)(g.z * 10) * 0.1f;
        std::cout << g.x << " " << g.y << " " << g.z << std::endl;
        g = g * DeltaTime();
            
        gfxm::quat qx = gfxm::angle_axis(g.x, object->Left());
        object->Rotate(qx);
        
        gfxm::quat qy = gfxm::angle_axis(g.y, object->Back());
        object->Rotate(qy);
        
        gfxm::quat qz = gfxm::angle_axis(g.z, object->Up());
        object->Rotate(qz);
        
        gfxm::vec3 projected_gravity(0.0f, 1.0f, 0.0f);
        projected_gravity = gfxm::inverse(object->GetTransform()) * gfxm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        gfxm::vec3 actual_gravity = imu.GetAccel();
        actual_gravity = gfxm::vec3(-actual_gravity.x, actual_gravity.z, actual_gravity.y);
        gfxm::vec3 mag = imu.GetMagnet();
        mag = gfxm::normalize(mag);
        projected_gravity = gfxm::normalize(projected_gravity);
        actual_gravity = gfxm::normalize(actual_gravity);
        
        gfxm::vec3 rotationAxis = gfxm::cross(actual_gravity, projected_gravity);
        float dot = gfxm::dot(projected_gravity, actual_gravity);

        float angle = (float)acos(fmax(-1.0f, fmin(dot, 1.0f)));
        gfxm::quat grav_correction = gfxm::angle_axis(angle, rotationAxis);
        
        //object->Rotation(imu.GetQuatAM());
        
        //object->Translate(velo * DeltaTime());
        
        //if(dot < .9f) object->Rotate(angle * DeltaTime(), rotationAxis);
        //object->LookAt(object->Position() + gfxm::vec3(0.0f, 0.0f, 1.0f), object->Back(), object->Up(), DeltaTime());

        accel->LookAt(accel->Position() + actual_gravity, accel->Up());
        magnet->LookAt(magnet->Position() + mag, magnet->Up());
        
        //object->LookAt(object->Position() + actual_gravity, object->Up(), object->Back(), 5.0f * DeltaTime());
        //object->LookAt(object->Position() + gfxm::vec3(0.0f, 0.0f, -1.0f), object->Forward(), object->Up(), DeltaTime());
        
        //object->Rotation(imu.GetQuat());
    }
    
    virtual void KeyDown(Au::Input::KEYCODE key)
    {
        if(key == Au::Input::KEY_Q)
        {
            object->Position(0.0f, 0.0f, 0.0f);
            velo = gfxm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    
    virtual void OnRender(Au::GFX::Device* device)
    {
        renderer->Render();
    }
private:
    SceneObject scene;
    Renderer* renderer;
    Camera* camera;
    Transform* object;
    gfxm::vec3 velo;
    Transform* accel;
    Transform* magnet;
    
    IMUDevice imu;
};

#endif
