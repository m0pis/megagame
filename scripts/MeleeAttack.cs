using UnityEngine;
using System.Collections;
using System.Collections.Generic;

//
// этот скрипт отвечает за систему ударов, типо база, ядро так сказать, и он не зависсит от врага или игрока, просто делает удар
//                                                           что дает возможность усложнять его как для врагов так и для игроков
//

public class MeleeAttack : MonoBehaviour
{
    [SerializeField] private int damage = 25; // дамаге(пока просто для вида)
    [SerializeField] private float attackSpeed = 1.5f; // ну аттак спид
    [SerializeField] private float attackTime = 0.3f; // время пока хитбокс будет активен
    [SerializeField] private float attackRadius = 1.2f; // радиус атаки 
    
    [SerializeField] private Transform attackPoint; // точка начала удара
    [SerializeField] private LayerMask targetLayer; // что может атаковать

    private bool canAttack = true;

    //
    // метод, который может вызвать другой скрипт (игрок или враг)
    //
    public void PerformAttack()
    {
        if (!canAttack) return;

        StartCoroutine(AttackCoroutine());
    }

    private IEnumerator AttackCoroutine()
    {
        canAttack = false;

        float timer = 0f;
        
        HashSet<Collider> hitTargets = new HashSet<Collider>(); // кого мы уже ударили этой атакой, чтобы не бить дважды за один замах

        while (timer < attackTime)
        {
            Collider[] colliders = Physics.OverlapSphere(attackPoint.position, attackRadius, targetLayer); // невидимая сфера в точке атаки

            foreach (var targetCollider in colliders)
            {
                if (!hitTargets.Contains(targetCollider))
                {
                    Debug.Log($"УДАР ПОЛУЧИЛ: {targetCollider.name}!");

                    //
                    // тут я будет логика получения урона, пока ее нету ^_^
                    //
                    // targetCollider.GetComponent<Health>()?.TakeDamage(damage);

                    hitTargets.Add(targetCollider);
                }
            }
            
            timer += Time.deltaTime;
            yield return null;
        }

        //
        // перезарядка атаки
        //
        yield return new WaitForSeconds(1f / attackSpeed);
        canAttack = true;
    }


    //
    // тут я для нашлядности области урара(покачто области) рисую сферу
    //
    private void OnDrawGizmosSelected()
    {
        if (attackPoint == null) return;

        Gizmos.color = Color.red;
        Gizmos.DrawWireSphere(attackPoint.position, attackRadius);
    }
}
