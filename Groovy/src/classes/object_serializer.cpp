#include "object_serializer.h"
#include "reflection.h"
#include "class_db.h"
#include "assets/asset_manager.h"

namespace utils
{
	static bool PropertyIsEqual(const GroovyProperty& prop, GroovyObject* obj1, GroovyObject* obj2)
	{
		void* objProp1 = (byte*)obj1 + prop.offset;
		void* objProp2 = (byte*)obj2 + prop.offset;
		uint32 objProp1ArrayCount = prop.arrayCount;
		uint32 objProp2ArrayCount = prop.arrayCount;

		if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
		{
			DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(prop.type);
			objProp1ArrayCount = dap.size(objProp1);
			objProp2ArrayCount = dap.size(objProp2);
			objProp1 = dap.data(objProp1);
			objProp2 = dap.data(objProp2);
		}

		if (objProp1ArrayCount != objProp2ArrayCount)
			return false;

		if (!(prop.flags & PROPERTY_FLAG_IS_COMPLEX))
		{
			return memcmp(objProp1, objProp2, reflectionUtils::GetPropertySize(prop.type) * objProp1ArrayCount) == 0;
		}
		else
		{
			switch (prop.type)
			{
				case PROPERTY_TYPE_STRING:
				{
					std::string* strPtr1 = (std::string*)objProp1;
					std::string* strPtr2 = (std::string*)objProp2;
					for (uint32 i = 0; i < objProp1ArrayCount; i++)
					{
						if (strPtr1[i] != strPtr2[i])
							return false;
					}
					return true;
				}
				break;

				case PROPERTY_TYPE_BUFFER:
				{
					Buffer* bufferPtr1 = (Buffer*)objProp1;
					Buffer* bufferPtr2 = (Buffer*)objProp2;
					for (uint32 i = 0; i < objProp1ArrayCount; i++)
					{
						if (bufferPtr1[i].size() != bufferPtr2[i].size())
							return false;
						if (memcmp(bufferPtr1[i].data(), bufferPtr2[i].data(), bufferPtr1[i].size()) != 0)
							return false;
					}
					return true;
				}
				break;

				case PROPERTY_TYPE_ASSET_REF:
				{
					AssetInstance** assetPtr1 = (AssetInstance**)objProp1;
					AssetInstance** assetPtr2 = (AssetInstance**)objProp2;
					for (uint32 i = 0; i < objProp1ArrayCount; i++)
					{
						if (assetPtr1[i] != assetPtr2[i])
							return false;
					}
					return true;
				}
				break;

				default:
					checkslowf(0, "Property comparison for this type not implemented");
					break;
			}
		}
	}

	static void SerializePropertyData(PropertyPack& pack, const GroovyProperty& prop, GroovyObject* obj)
	{
		void* objProp = (byte*)obj + prop.offset;
		uint32 objPropArrayCount = prop.arrayCount;

		if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
		{
			DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(prop.type);
			objPropArrayCount = dap.size(objProp);
			objProp = dap.data(objProp);
		}

		PropertyDesc& desc = pack.desc.emplace_back();
		desc.arrayCount = objPropArrayCount;
		desc.classProp = &prop;

		if (!(prop.flags & PROPERTY_FLAG_IS_COMPLEX))
		{
			size_t dataWidth = reflectionUtils::GetPropertySize(prop.type) * objPropArrayCount;
			pack.data.push_bytes(objProp, dataWidth);
			desc.sizeBytes = dataWidth;
		}
		else
		{
			switch (prop.type)
			{
				case PROPERTY_TYPE_STRING:
				{
					std::string* strPtr = (std::string*)objProp;
					for (uint32 i = 0; i < objPropArrayCount; i++)
					{
						pack.data.push<std::string>(*strPtr);
						desc.sizeBytes += strPtr->length() + 1;
						strPtr++;
					}
				}
				break;

				case PROPERTY_TYPE_BUFFER:
				{
					Buffer* bufferPtr = (Buffer*)objProp;
					for (uint32 i = 0; i < objPropArrayCount; i++)
					{
						pack.data.push<size_t>(bufferPtr->size());
						pack.data.push_bytes(bufferPtr->data(), bufferPtr->size());
						desc.sizeBytes += sizeof(size_t) + bufferPtr->size();
						bufferPtr++;
					}
				}
				break;

				case PROPERTY_TYPE_ASSET_REF:
				{
					AssetInstance** assetPtr = (AssetInstance**)objProp;
					for (uint32 i = 0; i < objPropArrayCount; i++)
					{
						AssetUUID uuid = 0;
						if (*assetPtr)
							uuid = (*assetPtr)->GetUUID();
						pack.data.push<AssetUUID>(uuid);
						assetPtr++;
					}
					desc.sizeBytes += sizeof(AssetUUID) * objPropArrayCount;
				}
				break;

				default:
					checkslowf(0, "Property serialization for this type not implemented");
					break;
			}
		}
	}

	static void DeserializePropertyData(const PropertyDesc& desc, const byte* data, GroovyObject* obj)
	{
		void* objProp = (byte*)obj + desc.classProp->offset;

		if (desc.classProp->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
		{
			DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(desc.classProp->type);
			dap.resize(objProp, desc.arrayCount);
			objProp = dap.data(objProp);
		}

		if (!(desc.classProp->flags & PROPERTY_FLAG_IS_COMPLEX))
		{
			memcpy(objProp, data, desc.sizeBytes);
		}
		else
		{
			switch (desc.classProp->type)
			{
				case PROPERTY_TYPE_STRING:
				{
					std::string* strPtr = (std::string*)objProp;
					for (uint32 i = 0; i < desc.arrayCount; i++)
					{
						*strPtr = (char*)data;
						data += strPtr->length() + 1;
						strPtr++;
					}
				}
				break;

				case PROPERTY_TYPE_BUFFER:
				{
					Buffer* bufferPtr = (Buffer*)objProp;
					for (uint32 i = 0; i < desc.arrayCount; i++)
					{
						size_t bufferSize = *(size_t*)data;
						bufferPtr->resize(bufferSize);
						data += sizeof(size_t);
						memcpy(bufferPtr->data(), data, bufferSize);
						data += bufferSize;
						bufferPtr++;
					}
				}
				break;

				case PROPERTY_TYPE_ASSET_REF:
				{
					AssetInstance** assetPtr = (AssetInstance**)objProp;
					for (uint32 i = 0; i < desc.arrayCount; i++)
					{
						*assetPtr = AssetManager::Get<AssetInstance>(*(AssetUUID*)data);
						data += sizeof(AssetUUID);
						assetPtr++;
					}
				}
				break;

				default:
					checkslowf(0, "Property deserialization for this type not implemented");
					break;
			}
		}
	}
}

void ObjectSerializer::CreatePropertyPack(GroovyObject* obj, GroovyObject* cdo, PropertyPack& outPack)
{
	checkslow(obj);

	extern ClassDB gClassDB;
	const std::vector<GroovyProperty>& props = gClassDB[obj->GetClass()];

	if (cdo)
	{
		checkslow(obj->GetClass() == cdo->GetClass());

		for (const GroovyProperty& p : props)
		{
			if (p.flags & PROPERTY_FLAG_NO_SERIALIZE)
				continue;

			if (utils::PropertyIsEqual(p, obj, cdo))
				continue;

			utils::SerializePropertyData(outPack, p, obj);
		}
	}
	else
	{
		for (const GroovyProperty& p : props)
		{
			if (p.flags & PROPERTY_FLAG_NO_SERIALIZE)
				continue;

			utils::SerializePropertyData(outPack, p, obj);
		}
	}
}

void ObjectSerializer::SerializePropertyPack(const PropertyPack& pack, DynamicBuffer& fileData)
{
	fileData.push<uint32>(pack.desc.size());									// property count
	size_t bufferOffset = 0;
	for (const auto& desc : pack.desc)
	{
		fileData.push(desc.classProp->name);									// property name
		fileData.push(desc.classProp->type);									// property type
		fileData.push(desc.arrayCount);											// property array count
		fileData.push(desc.sizeBytes);											// property data size
		fileData.push_bytes(pack.data.data() + bufferOffset, desc.sizeBytes);	// binary data
		bufferOffset += desc.sizeBytes;
	}
}

void ObjectSerializer::DeserializePropertyPack(GroovyClass* gClass, BufferView& fileData, PropertyPack& outPack)
{
	checkslow(gClass);

	if (!fileData.remaining())
		return;

	uint32 propCount = fileData.read<uint32>();

	extern ClassDB gClassDB;

	for (uint32 i = 0; i < propCount; i++)
	{
		std::string name = fileData.read<std::string>();
		EPropertyType type = fileData.read<EPropertyType>();
		uint32 arrayCount = fileData.read<uint32>();
		size_t sizeBytes = fileData.read<size_t>();

		const GroovyProperty* classProp = gClassDB.FindProperty(gClass, name);

		if (classProp)
		{
			// check property "signature"
			bool compatiblePropSignature = classProp->type == type;
			if (!(classProp->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY))
				compatiblePropSignature = compatiblePropSignature && classProp->arrayCount == arrayCount;

			if (classProp && compatiblePropSignature)
			{
				PropertyDesc& desc = outPack.desc.emplace_back();
				desc.classProp = classProp;
				desc.arrayCount = arrayCount;
				desc.sizeBytes = sizeBytes;

				outPack.data.push_bytes(fileData.seek(), sizeBytes);
			}
		}
		else
		{
			// TODO: Warning: property not found, class changed, please sanitize blueprint!
		}

		fileData.advance(sizeBytes);
	}
}

void ObjectSerializer::DeserializePropertyPackData(const PropertyPack& pack, GroovyObject* obj)
{
	checkslow(obj);

	const byte* data = pack.data.data();

	for (const PropertyDesc& desc : pack.desc)
	{
		utils::DeserializePropertyData(desc, data, obj);
		data += desc.sizeBytes;
	}
}
