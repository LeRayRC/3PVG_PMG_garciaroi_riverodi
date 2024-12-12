-- entity = ECS_CreateEntity()

print("Im entity " .. ENTITY_ID)

result = ECS_AddComponent_Transform(ENTITY_ID);
if result == 1 then
    print("Transform Component Added")
end
result = ECS_AddComponent_Render(ENTITY_ID);
if result == 1 then
    print("Render Component Added")
end





