using UnityEngine;

//
// этот скрипт простой хэндлер, он ждет нажатия кнопки атаки и потом передает в другой скрипт MeleeAttack что нада БИТЬ ПО МОРДЕ
//

[RequireComponent(typeof(MeleeAttack))]
public class WeaponSystem : MonoBehaviour
{
    private MeleeAttack meleeAttack;

    private void Awake()
    {
        meleeAttack = GetComponent<MeleeAttack>();
    }

    void Update()
    {
        if (Input.GetMouseButtonDown(0))
        {
            meleeAttack.PerformAttack();
        }
    }
}
