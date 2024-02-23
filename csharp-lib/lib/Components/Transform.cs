using System;
using System.Runtime.CompilerServices;

namespace Vault
{
    public class TVector3 : Vector3
    {
        public string ID;
        public string what_is;
        public TVector3(Vector2 vector, float z, string ID, string what_is) : base(vector, z)
        {
            this.ID = ID;
            this.what_is = what_is;
        }

        public TVector3(float x, Vector2 vector, string ID, string what_is) : base(x, vector)
        {
            this.ID = ID;
            this.what_is = what_is;
        }

        public TVector3(float x, float y, float z, string ID, string what_is) : base(x, y, z)
        {
            this.ID = ID;
            this.what_is = what_is;
        }

        public new float x
        {
            get { return base.x; }
            set
            {
                InternalCalls.Transform_SetField(ID, what_is, Math.ToRadians(value), Math.ToRadians(y), Math.ToRadians(z));
                base.x = value;
            }
        }

        public new float y
        {
            get { return base.y; }
            set
            {
                InternalCalls.Transform_SetField(ID, what_is, Math.ToRadians(x), Math.ToRadians(value), Math.ToRadians(z));
                base.y = value;
            }
        }

        public new float z
        {
            get { return base.z; }
            set
            {
                InternalCalls.Transform_SetField(ID, what_is, Math.ToRadians(x), Math.ToRadians(y), Math.ToRadians(value));
                base.z = value;
            }
        }
    }


    public class Transform : Component
    {

        public TVector3 position
        {
            get
            {
                InternalCalls.Transform_GetPosition(Entity.ID, out string result);
                string[] split = result.Split(" ");
                return new TVector3(Math.ToDegrees(float.Parse(split[0])), Math.ToDegrees(float.Parse(split[1])), Math.ToDegrees(float.Parse(split[2])), Entity.ID, "position");
            }
            set
            {
                InternalCalls.Transform_SetField(Entity.ID, "position", Math.ToRadians(value.x), Math.ToRadians(value.y), Math.ToRadians(value.z));
            }
        }
        public TVector3 rotation
        {
            get
            {
                InternalCalls.Transform_GetRotation(Entity.ID, out string result);
                string[] split = result.Split(" ");
                return new TVector3(Math.ToDegrees(float.Parse(split[0])), Math.ToDegrees(float.Parse(split[1])), Math.ToDegrees(float.Parse(split[2])), Entity.ID, "rotation");
            }
            set
            {
                InternalCalls.Transform_SetField(Entity.ID, "rotation", Math.ToRadians(value.x), Math.ToRadians(value.y), Math.ToRadians(value.z));
            }
        }
        public TVector3 scale
        {
            get
            {
                InternalCalls.Transform_GetScale(Entity.ID, out string result);
                string[] split = result.Split(" ");
                return new TVector3(Math.ToDegrees(float.Parse(split[0])), Math.ToDegrees(float.Parse(split[1])), Math.ToDegrees(float.Parse(split[2])), Entity.ID, "scale");
            }
            set
            {
                InternalCalls.Transform_SetField(Entity.ID, "scale", Math.ToRadians(value.x), Math.ToRadians(value.y), Math.ToRadians(value.z));
            }
        }
    }
}