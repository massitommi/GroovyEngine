#include "object_serializer.h"
#include "class_db.h"
#include "assets/asset_manager.h"

static void DeserializePropertyData(const PropertyDesc& desc, byte* data, GroovyObject* obj)
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
		}
	}
}

void ObjectSerializer::DeserializeOntoObject(const std::vector<PropertyDesc>& desc, byte* data, GroovyObject* obj)
{
	for (const PropertyDesc& d : desc)
	{
		DeserializePropertyData(d, data, obj);
		data += d.sizeBytes;
	}
}

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
					pack.data.push(*strPtr);
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
		}
	}
}

void ObjectSerializer::CreatePropertyPack(PropertyPack& pack, GroovyObject* obj, GroovyObject* cdo)
{
	extern ClassDB gClassDB;

	check(obj && cdo);
	check(obj->GetClass() == cdo->GetClass());

	const std::vector<GroovyProperty>& props = gClassDB[obj->GetClass()];

	for (const GroovyProperty& p : props)
	{
		if (p.flags & PROPERTY_FLAG_NO_SERIALIZE)
			continue;

		if (PropertyIsEqual(p, obj, cdo))
			continue;

		SerializePropertyData(pack, p, obj);
	}
}

void ObjectSerializer::SerializeSimpleObject(GroovyObject* obj, GroovyObject* cdo, DynamicBuffer& fileData)
{
	PropertyPack pack;
	CreatePropertyPack(pack, obj, cdo);

	fileData.push<uint32>(pack.desc.size());									// number of properties

	size_t bufferOffset = 0;

	for(const auto& desc : pack.desc)
	{
		fileData.push(desc.classProp->name);									// property name
		fileData.push(desc.arrayCount);											// property array count
		fileData.push(desc.sizeBytes);											// property data size
		fileData.push_bytes(pack.data.data() + bufferOffset, desc.sizeBytes);	// binary data
		bufferOffset += desc.sizeBytes;
	}
}

void ObjectSerializer::DeserializeSimpleObject(GroovyObject* obj, BufferView fileData)
{
	check(obj);

	extern ClassDB gClassDB;

	uint32 propertyCount = fileData.read<uint32>();

	PropertyPack pack;
	pack.desc.reserve(propertyCount);

	for (uint32 i = 0; i < propertyCount; i++)
	{
		PropertyDesc desc;
		std::string propName = fileData.read<std::string>();
		desc.arrayCount = fileData.read<uint32>();
		desc.sizeBytes = fileData.read<size_t>();

		const GroovyProperty* classProp = gClassDB.FindProperty(obj->GetClass(), propName);
		if (classProp)
		{
			desc.classProp = classProp;
			pack.data.push_bytes(fileData.seek(), desc.sizeBytes);
			pack.desc.push_back(desc);
		}
		else
		{
			// log or something
		}
		
		fileData.advance(desc.sizeBytes);
	}

	DeserializeOntoObject(pack.desc, pack.data.data(), obj);
}
