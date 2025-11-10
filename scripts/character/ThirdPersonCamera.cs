using UnityEngine;

//
// скрипт камеры, с таргет локом, проверкой коллизийЮ а также сглаживание движений и слежение за игроком
//

public class ThirdPersonCamera : MonoBehaviour
{
    [SerializeField] private Transform target;
    private Transform lockOnTarget;
    [SerializeField] private float mouseSensitivity = 3.0f;
    [SerializeField] private Vector2 pitchMinMax = new Vector2(-40, 85);
    [SerializeField][Range(0.01f, 1f)] private float mouseSmoothing = 0.1f;
    [SerializeField] private bool autoAlignCamera = true;
    [SerializeField] private float autoAlignSpeed = 2.0f;
    [SerializeField] private float desiredDistance = 5.0f;
    [SerializeField] private float zoomSpeed = 5.0f;
    [SerializeField] private Vector2 distanceMinMax = new Vector2(1.5f, 10.0f);
    [SerializeField] private Vector3 followOffset = new Vector3(0, 1.5f, 0);
    [SerializeField][Range(0.01f, 0.5f)] private float followSmoothTime = 0.1f;
    [SerializeField] private LayerMask collisionLayerMask;
    [SerializeField] private float cameraRadius = 0.2f;
    [SerializeField] private float collisionReturnSpeed = 9f;

    private Vector3 targetPivotPosition;
    private Vector3 pivotVelocity;

    private float yaw;
    private float pitch;
    private Vector2 smoothedMouseInput;
    private Vector2 mouseInputVelocity;

    private float currentDistance;

    void Start()
    {
        if (target == null)
        {
            Debug.LogError("none target");
            enabled = false;
            return;
        }

        currentDistance = desiredDistance;
        targetPivotPosition = target.position + followOffset;

        yaw = target.eulerAngles.y;
        pitch = 20f;

        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
    }

    public void SetLockOnTarget(Transform newTarget)
    {
        lockOnTarget = newTarget;
    }

    void LateUpdate()
    {
        targetPivotPosition = Vector3.SmoothDamp(targetPivotPosition, target.position + followOffset, ref pivotVelocity, followSmoothTime);

        if (lockOnTarget == null)
        {
            Vector2 mouseInput = new Vector2(Input.GetAxis("Mouse X"), Input.GetAxis("Mouse Y"));

            smoothedMouseInput = Vector2.SmoothDamp(smoothedMouseInput, mouseInput, ref mouseInputVelocity, mouseSmoothing);

            yaw += smoothedMouseInput.x * mouseSensitivity;
            pitch -= smoothedMouseInput.y * mouseSensitivity;

            if (autoAlignCamera && Input.GetAxisRaw("Vertical") > 0.1f)
            {
                float playerForwardYaw = target.eulerAngles.y;
                yaw = Mathf.LerpAngle(yaw, playerForwardYaw, Time.deltaTime * autoAlignSpeed);
            }
        }
        else // лок таргета
        {
            Quaternion lookRotation = Quaternion.LookRotation(lockOnTarget.position - targetPivotPosition);
            float targetYaw = lookRotation.eulerAngles.y;
            float targetPitch = lookRotation.eulerAngles.x;

            yaw = Mathf.LerpAngle(yaw, targetYaw, Time.deltaTime * 10f);
            pitch = Mathf.LerpAngle(pitch, targetPitch, Time.deltaTime * 10f);
        }

        pitch = Mathf.Clamp(pitch, pitchMinMax.x, pitchMinMax.y);

        Quaternion finalRotation = Quaternion.Euler(pitch, yaw, 0);
        Vector3 desiredPosition = targetPivotPosition - finalRotation * Vector3.forward * desiredDistance;

        HandleZoom();
        HandleCollisions(desiredPosition);

        transform.position = targetPivotPosition - finalRotation * Vector3.forward * currentDistance;
        transform.rotation = finalRotation;
    }

    private void HandleZoom()
    {
        desiredDistance = Mathf.Clamp(desiredDistance - Input.GetAxis("Mouse ScrollWheel") * zoomSpeed, distanceMinMax.x, distanceMinMax.y);
    }

    private void HandleCollisions(Vector3 desiredCameraPosition)
    {
        if (Physics.Linecast(targetPivotPosition, desiredCameraPosition, out RaycastHit hit, collisionLayerMask))
        {
            currentDistance = Vector3.Distance(targetPivotPosition, hit.point) - cameraRadius;
        }
        else
        {
            currentDistance = Mathf.Lerp(currentDistance, desiredDistance, Time.deltaTime * collisionReturnSpeed);
        }
        currentDistance = Mathf.Max(currentDistance, 0.1f);
    }
}
